#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Bezier1D.h"

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
	
	AddTexture("../res/textures/box0.bmp",false);
	// AddShape(Cube,-1,TRIANGLES);

	// TODO what is the diff?
	int segNum = 6;
	int res = 6;
	Bezier1D *bezier = new Bezier1D(segNum, res, 1);
	for(int i = 0; i < segNum; i++){
		for(int j = 0; j < 4; j++){
			glm::vec4 curr_point = bezier->GetControlPoint(i, j);
			AddShape(Octahedron, -1, TRIANGLES);
			// value = (((number - min_value) * (2 + 2)) / (max_value - min_value)) - 2
			float x_offset = (((curr_point[0] - 0) * (2)) / (16.f - 0));
			float y_offset = (((curr_point[1] - 0) * (2)) / (2.f - 0));
			printf("\n\ncurr point : (%f, %f, %f, %f\n", curr_point[0], curr_point[1], curr_point[2], curr_point[3]);
			printf("translate vec: (%f, %f, %f)\n\n", 0 + x_offset, 0 + y_offset, curr_point[2]);
			shapes[i + j]->MyTranslate(glm::vec3(0 + x_offset, 0 + y_offset, curr_point[2]), 0);
		}
	}
	shapes.push_back(bezier);
	// shapes.push_back(new Bezier1D(6, 6, 1));

	
	pickedShape = 0;
	
	SetShapeTex(0,0);
	MoveCamera(0,zTranslate,10);
	pickedShape = -1;
	
	//ReadPixel(); //uncomment when you are reading from the z-buffer
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
