// ???
#include "game.h"
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"

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

	std::string fileName = "../res/textures/lena256.jpg";
	int width, height, numComponents;
	unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4); //extract data from the image
	
	
	int width2 = width * 2;
	int height2 = height * 2;
	unsigned char* halftone_data = new unsigned char[width2 * height2 * 4];

	for (int i = 0; i < width2 * height2 * 4; i++) {
		halftone_data[i] = 0;
	}

	halftone(data, halftone_data, width2, height2);
	print_matrix(halftone_data, width2, height2);


	//AddTexture("../res/textures/lena256.jpg",false);
	AddTexture(width2, height2, halftone_data);

	stbi_image_free(data);

	AddShape(Plane,-1,TRIANGLES);
	
	pickedShape = 0;
	
	SetShapeTex(0,0);
	MoveCamera(0,zTranslate,1.5);
	pickedShape = -1;
	
	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::edge_detection(int width, int height, unsigned char* data)
{
}

void Game::halftone_pixel(unsigned char* data, unsigned char* new_data, int pixel_num, int width, std::vector<std::vector<unsigned char>> halftone_patterns)
{
	int row_num_in_data = pixel_num / (4 * width / 2);
	int column_num_in_data = (pixel_num % (4 * width / 2)) / 4;

	int row_num_in_new_data = 2 * row_num_in_data;
	int column_num_in_new_data = 2 * column_num_in_data;
	
	for (int color = 0; color < 4; color++) {

		int halftone_pattern_index = ((float)data[pixel_num + color]/256) * 5; // this is the index of the halftone option for the ith color of this pixel (e.g. if the value is under 64 the option will be 0)
		/*if(halftone_pattern_index == 4)
			std::cout << halftone_pattern_index << std::endl;*/

		std::vector<unsigned char> halftone_pattern = halftone_patterns[halftone_pattern_index];

		new_data[4 * width * row_num_in_new_data + 4 * column_num_in_new_data + color] = halftone_pattern[0];
		//new_data[2 * row_num_in_data + 2 * column_num_in_data] = halftone_pattern[0];
		//std::cout << (int)new_data[2 * row_num_in_data + 2 * column_num_in_data] << " ";
		
		new_data[4 * width * row_num_in_new_data + 4 * column_num_in_new_data + color + 4] = halftone_pattern[1];
		//new_data[2 * row_num_in_data + 2 * column_num_in_data + 4] = halftone_pattern[1];
		//std::cout << (int)new_data[2 * row_num_in_data + 2 * column_num_in_data] << std::endl;

		new_data[4 * width * (row_num_in_new_data + 1) + 4 * column_num_in_new_data + color] = halftone_pattern[2];
		//new_data[2 * row_num_in_data + 2 * column_num_in_data + 4 * width] = halftone_pattern[2];
		//std::cout << (int)new_data[2 * row_num_in_data + 2 * column_num_in_data + 4 * width] << " ";
		
		new_data[4 * width * (row_num_in_new_data + 1) + 4 * column_num_in_new_data + color + 4] = halftone_pattern[3];
		//new_data[2 * row_num_in_data + 2 * column_num_in_data + 4 * width + 4] = halftone_pattern[3];
		//std::cout << (int)new_data[2 * row_num_in_data + 2 * column_num_in_data + 4 * width + 4] << "\n";
	}
}

void Game::halftone(unsigned char* data, unsigned char* new_data, int width, int height)
{
	
	/*std::cout << "width = " << *width << std::endl;
	*width *= 4;
	std::cout << "width = " << *width << std::endl;
	*height *= 4;*/

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
		/*if (i > 1)
			break;*/
	}
	/*for (int i = 0; i < height * width * 4; i++) {
		std::cout << (int)new_data[i] << " ";
	}*/
}

void Game::floyd_steinberg(int width, int height, unsigned char* data)
{
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
