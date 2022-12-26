#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

//parameters:
float RUBIKS_CUBE_SIZE = 3.f;
float CUBE_SIZE = 3.f;
vector<vector<vector<Shape*>>> cube;

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
	cube = vector<vector<vector<Shape*>>>(RUBIKS_CUBE_SIZE, face);

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
				float offset = CUBE_SIZE * (RUBIKS_CUBE_SIZE-1) / 2;
				shapes[shape_indx]->MyTranslate(glm::vec3(k*CUBE_SIZE - offset, j*CUBE_SIZE - offset, -1*(i*CUBE_SIZE - offset)), 0);

				shape_indx++;
			}
		}
	}

	// AddShape(Plane, -1, TRIANGLES);
	// shapes[shape_indx]->MyScale(glm::vec3(8, 8, 8));

	// AddShape(Plane, -1, TRIANGLES);
	// shapes[shape_indx+1]->MyScale(glm::vec3(8, 8, 8));
	// shapes[shape_indx+1]->MyRotate(90, glm::vec3(1, 0, 0), 0);
	
	// AddShape(Plane, -1, TRIANGLES);
	// shapes[shape_indx+2]->MyScale(glm::vec3(8, 8, 8));
	// shapes[shape_indx+2]->MyRotate(90, glm::vec3(0, 1, 0), 0);

	pickedShape = 0;
	
	MoveCamera(0,zTranslate,55);
	MoveCamera(0,xTranslate,12);
	MoveCamera(0,yTranslate,10);

	pickedShape = -1;

	// shapes[0]->MyScale(glm::vec3(1.5, 1.5, 1.5));
	// shapes[0]->MyTranslate(glm::vec3(0.0, 2.0, 0.0), 0);
	
	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::my_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Game *scn = (Game*)glfwGetWindowUserPointer(window);

	if(action == GLFW_PRESS)
	{
		switch (key)
		{
			case GLFW_KEY_UP:
				/*
					- translate each cube to the origin (0,0,0) - lets call this vector v
					- rotate the cube the desired amount - lets call the rotation matrix r
					- translate the cube with the argument r*-v or -v*r (the rotated vector -v)
				*/
				break;
			case GLFW_KEY_DOWN:
				
				break;

		default:
			break;
		}
	}
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
