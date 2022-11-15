// ???
#include "game.h"
#include <iostream>
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
	
	AddTexture("../res/textures/lena256.jpg",false);
	// AddTexture("../res/textures/lena256.jpg", "edge_detection", 0);

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

void Game::halftone_pixel(unsigned char* data, unsigned char* new_data, int pixel_num, int width, std::vector<std::vector<unsigned char>> halftone_options)
{
	for (int i = 0; i < 4; i++) {
		int data_inti = (int)data[pixel_num + i];
		int option = (((int)data[pixel_num + i]) / 4) / (256 / 4); // this is the index of the halftone option for the ith color of this pixel (e.g. if the value is under 64 the option will be 0)
		std::vector<unsigned char> halftone_option = halftone_options[option];
		new_data[2 * pixel_num] = halftone_option[0];
		new_data[2 * (pixel_num + 4)] = halftone_option[1];
		new_data[2 * pixel_num + 4 * width] = halftone_option[2];
		new_data[2 * (pixel_num + 4) + 4 * width] = halftone_option[3];
	}
}

void Game::halftone(int* width, int* height, unsigned char* data)
{
	unsigned char* new_data = new unsigned char[*width * *height * 4]; //TODO: memory leaks!!

	std::vector<std::vector<unsigned char>> halftone_options
	{
		{(unsigned char)0, (unsigned char)0, (unsigned char)0, (unsigned char)0},
		{(unsigned char)0, (unsigned char)0, (unsigned char)255, (unsigned char)0},
		{(unsigned char)0, (unsigned char)255, (unsigned char)255, (unsigned char)0},
		{(unsigned char)0, (unsigned char)255, (unsigned char)255, (unsigned char)255},
		{(unsigned char)255, (unsigned char)255, (unsigned char)255, (unsigned char)255},
	};

	for (int i = 0; i < *width * *height * 4; i++) {
		halftone_pixel(data, new_data, i, *width, halftone_options);
	}

	std::cout << "got here" << std::endl;

	/*for (int i = 0; i < vec.size(); i++) {
		std::cout << vec[i] << std::endl;
	}*/

}

void Game::floyd_steinberg(int width, int height, unsigned char* data)
{

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
