// ???
#include "game.h"
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <map>

static void printMat(const glm::mat4 mat)
{
	std::cout<<" matrix:"<<std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout<< mat[j][i]<<" ";
		std::cout<<std::endl;
	}
}

Game::Game() : Scene()
{
}

Game::Game(float angle ,float relationWH, float near1, float far1) : Scene(angle,relationWH,near1,far1)
{ 	
}

void Game::Init()
{		
	AddShader("../res/shaders/pickingShader");	
	AddShader("../res/shaders/basicShader");

	AddShape(Plane, -1, TRIANGLES);

	//add grayscale texture
	std::string fileName = "../res/textures/lena256.jpg";
	int width, height, numComponents;
	unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4); //extract data from the image
	AddTexture(fileName, false);
	//set grayscale shape
	/*AddShape(Plane, -1, TRIANGLES);
	pickedShape = 0;
	SetShapeTex(0, 0);*/

	//add texture for edge detection
	unsigned char* edge_detection_data = new unsigned char[width * height * 4];
	edge_detection(data, edge_detection_data, width, height);
	AddTexture(width, height, edge_detection_data);
	delete[] edge_detection_data;
	//set edge detection shape
	/*AddShape(Plane, -1, TRIANGLES);
	pickedShape = 1;
	SetShapeTex(1, 1);*/
	
	//add texture for halftone
	int width2 = width * 2;
	int height2 = height * 2;
	unsigned char* halftone_data = new unsigned char[width2 * height2 * 4];
	halftone(data, halftone_data, width2, height2);
	AddTexture(width2, height2, halftone_data);
	delete[] halftone_data;
	//set halftone shape
	/*AddShape(Plane, -1, TRIANGLES);
	pickedShape = 2;
	SetShapeTex(2, 2);*/

	//add texture for floyd-steinberg
	unsigned char* floyd_steinberg_data = new unsigned char[width * height * 4];
	floyd_steinberg(data, floyd_steinberg_data, width, height);
	AddTexture(width, height, floyd_steinberg_data);
	delete[] floyd_steinberg_data;
	//set floyd-steinberg shape
	/*AddShape(Plane, -1, TRIANGLES);
	pickedShape = 3;
	SetShapeTex(3, 3);*/
	
	MoveCamera(0,zTranslate,1.5);
	pickedShape = -1;
	
	stbi_image_free(data);

	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::edge_detection(unsigned char* data, unsigned char* new_data, int width, int height)
{
	for (int i = 0; i < width * height * 4; i++)
		new_data[i] = data[i];
}

void Game::halftone_pixel(unsigned char* data, unsigned char* new_data, int pixel_num, int width, std::vector<std::vector<unsigned char>>& halftone_patterns)
{
	int row_num_in_data = pixel_num / (4 * width / 2);
	int column_num_in_data = (pixel_num % (4 * width / 2)) / 4;

	int row_num_in_new_data = 2 * row_num_in_data;
	int column_num_in_new_data = 2 * column_num_in_data;
	
	for (int color = 0; color < 4; color++) {

		int halftone_pattern_index = ((float)data[pixel_num + color]/256) * 5; // this is the index of the halftone option for the ith color of this pixel (e.g. if the value is under 64 the option will be 0)

		std::vector<unsigned char> halftone_pattern = halftone_patterns[halftone_pattern_index];

		new_data[4 * width * row_num_in_new_data + 4 * column_num_in_new_data + color] = halftone_pattern[0];		
		new_data[4 * width * row_num_in_new_data + 4 * column_num_in_new_data + color + 4] = halftone_pattern[1];
		new_data[4 * width * (row_num_in_new_data + 1) + 4 * column_num_in_new_data + color] = halftone_pattern[2];
		new_data[4 * width * (row_num_in_new_data + 1) + 4 * column_num_in_new_data + color + 4] = halftone_pattern[3];

	}
}

void Game::halftone(unsigned char* data, unsigned char* new_data, int width, int height)
{

	std::vector<std::vector<unsigned char>> halftone_patterns
	{
		{0, 0, 0, 0},
		{0, 0, 255, 0},
		{0, 255, 255, 0},
		{0, 255, 255, 255},
		{255, 255, 255, 255},
	};
	
	for (int i = 0; i < width/2 * height/2 * 4; i+=4) {
		halftone_pixel(data, new_data, i, width, halftone_patterns);
	}
	
}

void Game::floyd_steinberg_pixel(std::vector<std::vector<float>>& new_data_float_values, int row_num, int column_num, int width, int height, std::vector<float>& colors)
{
	float original_color = new_data_float_values[row_num][column_num];
	float new_color = colors[(int)(original_color / 16)];
	float diff = original_color - new_color;

	new_data_float_values[row_num][column_num] = new_color;
	
	if (column_num + 4 < width * 4)
		new_data_float_values[row_num][column_num + 4] += diff * 7 / 16;
	
	if (row_num + 1 < height) {
		if (column_num - 4 >= 0)
			new_data_float_values[row_num + 1][column_num - 4] += diff * 3 / 16;

		new_data_float_values[row_num + 1][column_num] += diff * 5 / 16;

		if (column_num + 4 < width * 4)
			new_data_float_values[row_num + 1][column_num + 4] += diff * 1 / 16;
	}
	
}

void Game::floyd_steinberg(unsigned char* data, unsigned char* new_data, int width, int height)
{
	int data_size = width * height * 4;

	std::vector<std::vector<float>> new_data_float_values(height, std::vector<float>(width * 4)); //a matrix representing the float values of new_data
	std::vector<std::vector<float>>& new_data_float_values_ref = new_data_float_values; //reference of the vector to pass into floyd_steinberg_pixel

	std::vector<float> colors(16);
	for (int i = 0; i < 16; i++) {
		colors[i] = ((float)i) * 256 / 16; //the 16 color options
	}
	
	for (int i = 0; i < data_size; i++) {
		int row_num = i / (4 * width);
		int column_num = (i % (4 * width)) / 4;
		int color = i % 4;

		new_data_float_values[row_num][column_num + color] = (float)((int)data[i]); //give the pixel it's original color
		floyd_steinberg_pixel(new_data_float_values_ref, row_num, column_num + color, width, height, colors); //give the pixel it's new color and propagate the error
		new_data[i] = (unsigned char)((int)new_data_float_values[row_num][column_num + color]); //convert the pixel's new color to unsigned char and enter it in new_data

	}
}

void Game::print_matrix(unsigned char* data, int width, int height)
{
	std::ofstream matrix_file;
	matrix_file.open("matrix_file.txt");

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width * 4; j+=4) {
			matrix_file << (int)data[i * width * 4 + j] << " " << (int)data[i * width * 4 + j + 1] << " " << (int)data[i * width * 4 + j + 2] << " " << (int)data[i * width * 4 + j + 3] << "   ";
		}
		matrix_file << "\n\n";
	}

	matrix_file.close();
}

void Game::Update(const glm::mat4 &MVP,const glm::mat4 &Model,const int  shaderIndx)
{
	Shader *s = shaders[shaderIndx];
	int r = ((pickedShape+1) & 0x000000FF) >>  0;
	int g = ((pickedShape+1) & 0x0000FF00) >>  8;
	int b = ((pickedShape+1) & 0x00FF0000) >> 16;
	s->Bind();
	s->SetUniformMat4f("MVP", MVP);
	s->SetUniformMat4f("Normal",Model);
	s->SetUniform4f("lightDirection", 0.0f , 0.0f, -1.0f, 0.0f);
	if(shaderIndx == 0)
		s->SetUniform4f("lightColor",r/255.0f, g/255.0f, b/255.0f,1.0f);
	else 
		s->SetUniform4f("lightColor",0.7f,0.8f,0.1f,1.0f);
	s->Unbind();
}

void Game::WhenRotate()
{
}

void Game::WhenTranslate()
{
}

void Game::Motion()
{
	if(isActive)
	{
	}
}

Game::~Game(void)
{
}
