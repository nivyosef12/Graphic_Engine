#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

//parameters:
int RUBIKS_CUBE_SIZE = 3;
int CUBE_SIZE = 3;

static void printMat(const glm::mat4 mat)
{
	cout<<" matrix:"<<endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			cout<< mat[j][i]<<" ";
		cout<<endl;
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
	
	AddTexture("../res/textures/plane.png",false);

	vector<Shape*> line(RUBIKS_CUBE_SIZE);
	vector<vector<Shape*>> face(RUBIKS_CUBE_SIZE, line);
	vector<vector<vector<Shape*>>> cube(RUBIKS_CUBE_SIZE, face);

	int shape_indx = 0;
	for (int i = 0; i < RUBIKS_CUBE_SIZE; i++) {
		for (int j = 0; j < RUBIKS_CUBE_SIZE; j++) {
			for (int k = 0; k < RUBIKS_CUBE_SIZE; k++) {
				if (i == 0 && j == 0 && k == 0) {
					AddShape(Cube, -1, TRIANGLES);
					SetShapeTex(0, 0);
				} else {
					AddShapeCopy(0, -1, TRIANGLES);
				}
				/*printf("i: %i, j: %i, k: %i\n", i, j, k);
				printf("shape index: %i\n", shape_indx);*/
				cube[k][j][i] = shapes[shape_indx];
				shapes[shape_indx]->MyTranslate(glm::vec3(k*CUBE_SIZE, j*CUBE_SIZE, -i*CUBE_SIZE), 0);

				shape_indx++;
			}
		}
	}
	
	pickedShape = 0;
	
	MoveCamera(0,zTranslate,40);
	MoveCamera(0,xTranslate,10);

	pickedShape = -1;

	// shapes[0]->MyScale(glm::vec3(1.5, 1.5, 1.5));
	// shapes[0]->MyTranslate(glm::vec3(0.0, 2.0, 0.0), 0);
	cube[1][1][0]->MyRotate(90, glm::vec3(0.0, 1.0, 0.0), 0);
	cube[1][1][0]->MyRotate(90, glm::vec3(0.0, 1.0, 0.0), 0);

	
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
