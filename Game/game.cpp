#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

//constants:
const double pi = 3.14159265358979323846;

//parameters:
float RUBIKS_CUBE_SIZE = 3.f;
float CUBE_SIZE = 2.f;
vector<vector<vector<Shape*>>> cube;
float ROTATION_ANGLE = pi / 4;
glm::mat4 cube_center_trans(1);
glm::mat4 cube_center_rot(1);
glm::mat4 cube_center_scl(1); 

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

	cube = make_cube();

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

void Game::my_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	Game *scn = (Game*)glfwGetWindowUserPointer(window);	
	if(action == GLFW_PRESS)
	{
		switch (key)
		{			
			case GLFW_KEY_UP:
			{
				// for (int i = 0; i < RUBIKS_CUBE_SIZE; i++) {
				// 	for (int j = 0; j < RUBIKS_CUBE_SIZE; j++) {
				// 		for (int k = 0; k < RUBIKS_CUBE_SIZE; k++) {
				// 			// glm::mat4 previous_trans = cube[k][j][i]->get_trans();
				// 			// glm::mat4 trans_mat = cube_center_trans - previous_trans;
				// 			// glm::vec3 trans_vec1(trans_mat[3][0], trans_mat[3][1], trans_mat[3][2]);
				// 			// cube[k][j][i]->MyTranslate(trans_vec1, 0);

				// 			// cube[k][j][i]->MyRotate(-45, glm::vec3(1, 0, 0), 0);

				// 			// float rotation[9] = {
				// 			// 	1, 0                  , 0                   ,
				// 			// 	0, cos(-ROTATION_ANGLE), -sin(-ROTATION_ANGLE),
				// 			// 	0, sin(-ROTATION_ANGLE),  cos(-ROTATION_ANGLE)
				// 			// };
				// 			// glm::mat3 rotation_mat = glm::make_mat3(rotation);
				// 			// glm::vec3 trans_vec2 = rotation_mat * -trans_vec1;
							
				// 			// cube[k][j][i]->MyTranslate(trans_vec2, 0);


				// 			float angle = -45;
				// 			glm::vec3 axis(1, 0, 0);
				// 			printf("before\n");
				// 			glm::mat4 previous_trans = cube[k][j][i]->get_trans();
				// 			printf("after\n");
				// 			glm::mat4 trans_mat1 = cube_center_trans - previous_trans;
				// 			glm::vec3 trans_vec1(trans_mat1[3][0], trans_mat1[3][1], trans_mat1[3][2]);
				// 			cube[k][j][i]->MyTranslate(trans_vec1, 0);

				// 			cube[k][j][i]->MyRotate(angle, axis, 0);

				// 			// float rotation[9] = {
				// 			// 	1, 0                  , 0                   ,
				// 			// 	0, cos(angle), -sin(angle),
				// 			// 	0, sin(angle),  cos(angle)
				// 			// };
				// 			// glm::mat3 rotation_mat = glm::make_mat3(rotation);
				// 			glm::mat4 trans_mat2 = glm::rotate(-trans_mat1, angle, axis);
				// 			glm::vec3 trans_vec2(trans_mat2[3][0], trans_mat2[3][1], trans_mat2[3][2]);
							
				// 			cube[k][j][i]->MyTranslate(trans_vec2, 0);

				// 		}
				// 	}
				// }

				scn->rotate_cube(scn, -45, glm::vec3(1, 0, 0));
			
				break;
			}
		
			case GLFW_KEY_DOWN:
				scn->rotate_cube(scn, 45, glm::vec3(1, 0, 0));
				break;
			
			case GLFW_KEY_RIGHT:
				scn->rotate_cube(scn, 45, glm::vec3(0, 1, 0));
				break;

			case GLFW_KEY_LEFT:
				scn->rotate_cube(scn, -45, glm::vec3(0, 1, 0));
				break;

		
		default:
			break;
		}
	}
}

void Game::rotate_cube(Game* scn, float angle, glm::vec3 axis)
{
	vector<vector<vector<Shape*>>> new_cube = scn->make_cube();
	for (int i = 0; i < RUBIKS_CUBE_SIZE; i++) {
		for (int j = 0; j < RUBIKS_CUBE_SIZE; j++) {
			for (int k = 0; k < RUBIKS_CUBE_SIZE; k++) {
				printf("before\n");
				glm::mat4 previous_trans = cube[k][j][i]->get_trans();
				printf("after\n");
				glm::mat4 trans_mat1 = cube_center_trans - previous_trans;
				glm::vec3 trans_vec1(trans_mat1[3][0], trans_mat1[3][1], trans_mat1[3][2]);
				cube[k][j][i]->MyTranslate(trans_vec1, 0);

				cube[k][j][i]->MyRotate(angle, axis, 0);

				// float rotation[9] = {
				// 	1, 0                  , 0                   ,
				// 	0, cos(angle), -sin(angle),
				// 	0, sin(angle),  cos(angle)
				// };
				// glm::mat3 rotation_mat = glm::make_mat3(rotation);
				glm::mat4 trans_mat2 = glm::rotate(-trans_mat1, angle, axis);
				glm::vec3 trans_vec2(trans_mat2[3][0], trans_mat2[3][1], trans_mat2[3][2]);
				
				cube[k][j][i]->MyTranslate(trans_vec2, 0);
			}
		}
	}
}

vector<vector<vector<Shape*>>> Game::make_cube()
{
	vector<Shape*> line(RUBIKS_CUBE_SIZE);
	vector<vector<Shape*>> face(RUBIKS_CUBE_SIZE, line);
	cube = vector<vector<vector<Shape*>>>(RUBIKS_CUBE_SIZE, face);

	return cube;
}

Game::~Game(void)
{
}
