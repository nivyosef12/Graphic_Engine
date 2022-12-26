#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

//constants:
const double pi = 3.14159265358979323846;

//parameters:
float RUBIKS_CUBE_SIZE = 5.f;
float CUBE_SIZE = 3.f;
// static vector<vector<vector<Shape*>>> rubicks_cube;
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

	rubicks_cube = make_cube();

	int shape_indx = 0;
	for (int z = 0; z < RUBIKS_CUBE_SIZE; z++) {
		for (int y = 0; y < RUBIKS_CUBE_SIZE; y++) {
			for (int x = 0; x < RUBIKS_CUBE_SIZE; x++) {
				if (x == 0 && y == 0 && z == 0) {
					AddShape(Cube, -1, TRIANGLES);
					SetShapeTex(0, 0);
				} else {
					AddShapeCopy(0, -1, TRIANGLES);
				}
				/*printf("i: %i, j: %i, k: %i\n", i, j, k);
				printf("shape index: %i\n", shape_indx);*/
				rubicks_cube[x][y][z] = shapes[shape_indx];
				float offset = CUBE_SIZE * (RUBIKS_CUBE_SIZE-1) / 2;
				shapes[shape_indx]->MyTranslate(glm::vec3(x*CUBE_SIZE - offset, y*CUBE_SIZE - offset, -1*(z*CUBE_SIZE - offset)), 0);

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
				// scn->rubicks_cube[0][0][0]->MyRotate(-45, glm::vec3(1, 0, 0), 0);
				scn->rotate_cube(-45, glm::vec3(1, 0, 0)); 
				//rotate 45 deg counter-clockwise around the "real world's" x-axis
				break;
		
			case GLFW_KEY_DOWN:
				// scn->rubicks_cube[0][0][0]->MyRotate(45, glm::vec3(1, 0, 0), 0);
				scn->rotate_cube(45, glm::vec3(1, 0, 0)); 
				//rotate 45 deg clockwise around the "real world's" x-axis
				break;
			
			case GLFW_KEY_RIGHT:
				scn->rotate_cube(45, glm::vec3(0, 1, 0)); 
				//rotate 45 deg clockwise around the "real world's" y-axis
				break;

			case GLFW_KEY_LEFT:
				scn->rotate_cube(-45, glm::vec3(0, 1, 0)); 
				//rotate 45 deg counter-clockwise around the "real world's" y-axis
				break;

		default:
			break;
		}
	}
}

void Game::rotate_cube(float angle, glm::vec3 axis)
{
	for (int z = 0; z < RUBIKS_CUBE_SIZE; z++) {
		for (int y = 0; y < RUBIKS_CUBE_SIZE; y++) {
			for (int x = 0; x < RUBIKS_CUBE_SIZE; x++) {
				Shape* cube = rubicks_cube[x][y][z];
				
				glm::mat4 previous_trans = cube->get_trans();
				glm::mat4 trans_mat1 = cube_center_trans - previous_trans; 
				glm::vec3 trans_vec1(trans_mat1[3][0], trans_mat1[3][1], trans_mat1[3][2]); //the translation matrix from the cube's position to the center of the rubick's cube
				cube->MyTranslate(trans_vec1, 0); //translate to the center of the rubick's cube

				glm::vec4 real_axis = glm::inverse(cube->get_rot()) * glm::vec4(axis, 1); //rotate the "real" x-axis by the opposite of the rotations that the cube did, this is the real x-axis as the cube sees it
				cube->MyRotate(angle, glm::vec3(real_axis), 0); 

				glm::mat4 trans_mat2 = glm::rotate(glm::mat4(1), angle, axis) * -trans_mat1; //take the reverse of trans_mat1 and rotate it
				glm::vec3 trans_vec2(trans_mat2[3][0], trans_mat2[3][1], trans_mat2[3][2]);							
				cube->MyTranslate(trans_vec2, 0); //translate to the final location
			}
		}
	}

	// //if a full 90 deg turn has been completed, switch the faces
	// vector<vector<vector<Shape*>>> new_rubicks_cube = make_cube();
	// if (true) {
	// 	char rotation_axis = 'z';
	// 	if (glm::all(glm::equal(axis, glm::vec3(1, 0, 0)))) {
	// 		rotation_axis = 'x';
	// 	} else if (glm::all(glm::equal(axis, glm::vec3(0, 1, 0)))) {
	// 		rotation_axis = 'y';
	// 	}

	// 	int num_of_rotations = 1;
	// 	if (angle > 0)
	// 		num_of_rotations = 3;
		
	// 	for (int z = 0; z < RUBIKS_CUBE_SIZE; z++) {
	// 		for (int y = 0; y < RUBIKS_CUBE_SIZE; y++) {
	// 			for (int x = 0; x < RUBIKS_CUBE_SIZE; x++) {
	// 				Shape* cube = rubicks_cube[x][y][z];

	// 				for (int i = 0; i < num_of_rotations; i++) { // if rotating clockwise, rotate counter-clockwise 3 times	
	// 					if (rotation_axis == 'x' && (z == 0 || z == RUBIKS_CUBE_SIZE-1 || y == 0 || y == RUBIKS_CUBE_SIZE-1)) {
	// 						new_rubicks_cube[x][RUBIKS_CUBE_SIZE - z][y] = cube;
							
	// 						// if (z == 0 || z == RUBIKS_CUBE_SIZE-1 || y == 0 || y == RUBIKS_CUBE_SIZE-1) {
	// 						// 	new_rubicks_cube[x][RUBIKS_CUBE_SIZE - z][y] = cube;
	// 						// } else {
	// 						// 	new_rubicks_cube[x][y][z] = cube;
	// 						// }
	// 					} else if (rotation_axis == 'y' && (z == 0 || z == RUBIKS_CUBE_SIZE-1 || x == 0 || x == RUBIKS_CUBE_SIZE-1)) {
	// 						new_rubicks_cube[RUBIKS_CUBE_SIZE - z][y][x] = cube;
							
	// 						// if (z == 0 || z == RUBIKS_CUBE_SIZE-1 || x == 0 || x == RUBIKS_CUBE_SIZE-1) {
	// 						// 	new_rubicks_cube[RUBIKS_CUBE_SIZE - z][y][x] = cube;
	// 						// } else {
	// 						// 	new_rubicks_cube[x][y][z] = cube;
	// 						// }
	// 					} else  if (rotation_axis == 'z' && (x == 0 || x == RUBIKS_CUBE_SIZE-1 || y == 0 || y == RUBIKS_CUBE_SIZE-1)) {
	// 						new_rubicks_cube[RUBIKS_CUBE_SIZE - y][x][z] = cube;

	// 						// if (x == 0 || x == RUBIKS_CUBE_SIZE-1 || y == 0 || y == RUBIKS_CUBE_SIZE-1) {
	// 						// 	new_rubicks_cube[RUBIKS_CUBE_SIZE - y][x][z] = cube;
	// 						// } else {
	// 						// 	new_rubicks_cube[x][y][z] = cube;
	// 						// }
	// 					} else {
	// 						new_rubicks_cube[x][y][z] = cube;
	// 					}
	// 				}			
	// 			}
	// 		}
	// 	}
	// }
}

vector<vector<vector<Shape*>>> Game::make_cube()
{
	vector<Shape*> line(RUBIKS_CUBE_SIZE);
	vector<vector<Shape*>> face(RUBIKS_CUBE_SIZE, line);
	rubicks_cube = vector<vector<vector<Shape*>>>(RUBIKS_CUBE_SIZE, face);

	return rubicks_cube;
}

Game::~Game(void)
{
}
