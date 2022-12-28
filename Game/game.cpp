#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// #include <map>

using namespace std;

//constants:
const double pi = 3.14159265358979323846;

//parameters:
float RUBIKS_CUBE_SIZE = 3.f;
float CUBE_SIZE = 2.f;
// static vector<vector<vector<Shape*>>> rubicks_cube;
float ROTATION_ANGLE = 45;
float WHOLE_CUBE_ROTATION_ANGLE = 45;
glm::mat4 cube_center_trans(1);
glm::mat4 cube_center_rot(1);
glm::mat4 cube_center_scl(1); 
map<Face, float> angles_rotated_relative;
map<Face, float> angles_rotated_absolute;
glm::vec3 cube_x_axis(1, 0, 0);	
glm::vec3 cube_y_axis(0, 1, 0);	
glm::vec3 cube_z_axis(0, 0, 1);

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
				rubicks_cube[x][y][z] = shapes[shape_indx];
				float offset = CUBE_SIZE * (RUBIKS_CUBE_SIZE-1) / 2;
				shapes[shape_indx]->MyTranslate(glm::vec3(x*CUBE_SIZE - offset, y*CUBE_SIZE - offset, -1*(z*CUBE_SIZE - offset)), 0);

				shape_indx++;
			}
		}
	}

	char axis_lst[] = {'x', 'y', 'z'}; 
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < RUBIKS_CUBE_SIZE; j++) {
			Face face = make_tuple(axis_lst[i], j);
			angles_rotated_relative[face] = 0;
			angles_rotated_absolute[face] = 0;
		}
	}

	pickedShape = 0;
	
	MoveCamera(0,zTranslate,45);
	MoveCamera(0,xTranslate,8);
	MoveCamera(0,yTranslate,8);

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

void Game::my_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	Game *scn = (Game*)glfwGetWindowUserPointer(window);
	
	if(action == GLFW_PRESS)
	{
		Shape* cube;
		switch (key)
		{			
			case GLFW_KEY_UP:
				WHOLE_CUBE_ROTATION_ANGLE = -1 * abs(WHOLE_CUBE_ROTATION_ANGLE);
				for (auto const& x: angles_rotated_absolute) {
					if (get<0>(x.first) == 'x')
						angles_rotated_absolute[x.first] += WHOLE_CUBE_ROTATION_ANGLE;
				}
				scn->rotate_cube(WHOLE_CUBE_ROTATION_ANGLE, glm::vec3(1, 0, 0)); 
				//rotate 45 deg counter-clockwise around the "real world's" x-axis
				break;
		
			case GLFW_KEY_DOWN:
				WHOLE_CUBE_ROTATION_ANGLE = abs(WHOLE_CUBE_ROTATION_ANGLE);
				for (auto const& x: angles_rotated_absolute) {
					if (get<0>(x.first) == 'x')
						angles_rotated_absolute[x.first] += WHOLE_CUBE_ROTATION_ANGLE;
				}
				scn->rotate_cube(WHOLE_CUBE_ROTATION_ANGLE, glm::vec3(1, 0, 0)); 
				//rotate 45 deg clockwise around the "real world's" x-axis
				break;
			
			case GLFW_KEY_RIGHT:
				WHOLE_CUBE_ROTATION_ANGLE = abs(WHOLE_CUBE_ROTATION_ANGLE);
				for (auto const& x: angles_rotated_absolute) {
					if (get<0>(x.first) == 'y')
						angles_rotated_absolute[x.first] += WHOLE_CUBE_ROTATION_ANGLE;
				}
				scn->rotate_cube(WHOLE_CUBE_ROTATION_ANGLE, glm::vec3(0, 1, 0)); 
				//rotate 45 deg clockwise around the "real world's" y-axis
				break;

			case GLFW_KEY_LEFT:
				WHOLE_CUBE_ROTATION_ANGLE = -1 * abs(WHOLE_CUBE_ROTATION_ANGLE);
				for (auto const& x: angles_rotated_absolute) {
					if (get<0>(x.first) == 'y')
						angles_rotated_absolute[x.first] += WHOLE_CUBE_ROTATION_ANGLE;
				}
				scn->rotate_cube(WHOLE_CUBE_ROTATION_ANGLE, glm::vec3(0, 1, 0)); 
				//rotate 45 deg counter-clockwise around the "real world's" y-axis
				break;

			case GLFW_KEY_C:
				WHOLE_CUBE_ROTATION_ANGLE = -1 * abs(WHOLE_CUBE_ROTATION_ANGLE);
				for (auto const& x: angles_rotated_absolute) {
					if (get<0>(x.first) == 'z')
						angles_rotated_absolute[x.first] += WHOLE_CUBE_ROTATION_ANGLE;
				}
				scn->rotate_cube(WHOLE_CUBE_ROTATION_ANGLE, glm::vec3(0, 0, 1)); 
				//rotate 45 deg clockwise around the "real world's" y-axis
				break;

			case GLFW_KEY_X:
				WHOLE_CUBE_ROTATION_ANGLE = abs(WHOLE_CUBE_ROTATION_ANGLE);
				for (auto const& x: angles_rotated_absolute) {
					if (get<0>(x.first) == 'z')
						angles_rotated_absolute[x.first] += WHOLE_CUBE_ROTATION_ANGLE;
				}
				scn->rotate_cube(WHOLE_CUBE_ROTATION_ANGLE, glm::vec3(0, 0, 1)); 
				//rotate 45 deg clockwise around the "real world's" y-axis
				break;
			case GLFW_KEY_R:
				angles_rotated_relative[make_tuple('x', RUBIKS_CUBE_SIZE-1)] += ROTATION_ANGLE;
				scn->rotate_face(ROTATION_ANGLE, cube_x_axis, RUBIKS_CUBE_SIZE-1, angles_rotated_relative);
				break;
			case GLFW_KEY_L:
				angles_rotated_relative[make_tuple('x', 0)] += ROTATION_ANGLE;
				scn->rotate_face(ROTATION_ANGLE, cube_x_axis, 0, angles_rotated_relative);
				break;
			case GLFW_KEY_U:
				angles_rotated_relative[make_tuple('y', RUBIKS_CUBE_SIZE-1)] += ROTATION_ANGLE;
				scn->rotate_face(ROTATION_ANGLE, cube_y_axis, RUBIKS_CUBE_SIZE-1, angles_rotated_relative);
				break;
			case GLFW_KEY_D:
				angles_rotated_relative[make_tuple('y', 0)] += ROTATION_ANGLE;
				scn->rotate_face(ROTATION_ANGLE, cube_y_axis, 0, angles_rotated_relative);
				break;
			case GLFW_KEY_B:
				angles_rotated_relative[make_tuple('z', RUBIKS_CUBE_SIZE-1)] += ROTATION_ANGLE;
				scn->rotate_face(ROTATION_ANGLE, cube_z_axis, RUBIKS_CUBE_SIZE-1, angles_rotated_relative);
				break;
			case GLFW_KEY_F:
				angles_rotated_relative[make_tuple('z', 0)] += ROTATION_ANGLE;
				scn->rotate_face(ROTATION_ANGLE, cube_z_axis, 0, angles_rotated_relative);
				break;
			case GLFW_KEY_SPACE:
				ROTATION_ANGLE *= -1;
				break;
			case GLFW_KEY_Z:
				ROTATION_ANGLE /= 2;
				WHOLE_CUBE_ROTATION_ANGLE /= 2;
				break;
			case GLFW_KEY_A:
				if (ROTATION_ANGLE <= 90)
					ROTATION_ANGLE *= 2;
				if (WHOLE_CUBE_ROTATION_ANGLE <= 90)
					WHOLE_CUBE_ROTATION_ANGLE *= 2;
				break;
			case GLFW_KEY_1:
				cube = scn->rubicks_cube[0][0][0];
				cube->MyRotate(ROTATION_ANGLE, cube_y_axis, 0);
				break;
			case GLFW_KEY_2:
				cube = scn->rubicks_cube[1][0][0];
				cube->MyRotate(ROTATION_ANGLE, cube_y_axis, 0);
				break;
			case GLFW_KEY_3:
				cube = scn->rubicks_cube[2][0][0];
				cube->MyRotate(ROTATION_ANGLE, cube_y_axis, 0);
				break;
			case GLFW_KEY_4:
				cube = scn->rubicks_cube[0][1][0];
				cube->MyRotate(ROTATION_ANGLE, cube_y_axis, 0);
				break;
			case GLFW_KEY_5:
				cube = scn->rubicks_cube[1][1][0];
				cube->MyRotate(ROTATION_ANGLE, cube_y_axis, 0);
				break;
			case GLFW_KEY_6:
				cube = scn->rubicks_cube[2][1][0];
				cube->MyRotate(ROTATION_ANGLE, cube_y_axis, 0);
				break;
			case GLFW_KEY_7:
				cube = scn->rubicks_cube[0][2][0];
				cube->MyRotate(ROTATION_ANGLE, cube_y_axis, 0);
				break;
			case GLFW_KEY_8:
				cube = scn->rubicks_cube[1][2][0];
				cube->MyRotate(ROTATION_ANGLE, cube_y_axis, 0);
				break;
			case GLFW_KEY_9:
				cube = scn->rubicks_cube[2][2][0];
				cube->MyRotate(ROTATION_ANGLE, cube_y_axis, 0);
				break;
			
		default:
			break;
		}
	}
}

void Game::change_cube_axes(glm::vec3 axis)
{
	glm::mat4 rotation_mat = glm::rotate(glm::mat4(1), WHOLE_CUBE_ROTATION_ANGLE, axis);
	cube_center_rot = rotation_mat * cube_center_rot;
	cube_x_axis = glm::vec3(rotation_mat * glm::vec4(cube_x_axis, 1));
	cube_y_axis = glm::vec3(rotation_mat * glm::vec4(cube_y_axis, 1));
	cube_z_axis = glm::vec3(rotation_mat * glm::vec4(cube_z_axis, 1));		
}

void Game::rotate_cube(float angle, glm::vec3 axis)
{
	//if a more than 45 deg turn has been completed, switch the faces

	float prev_angle = angle;
	if (angle > 90) {
		angle = 90;
	} else if (angle < -90) {
		angle = -90;
	}

	char rotation_axis = 'z';
	if (glm::all(glm::equal(axis, glm::vec3(1, 0, 0)))) {
		rotation_axis = 'x';
	} else if (glm::all(glm::equal(axis, glm::vec3(0, 1, 0)))) {
		rotation_axis = 'y';
	}
	
	bool rotated_data_structure = true;
	for (int z = 0; z < RUBIKS_CUBE_SIZE; z++) {
		rotated_data_structure &= rotate_face(angle, axis, z, angles_rotated_absolute);
	}

	change_cube_axes(axis);

	if (rotated_data_structure) {
		float sign = angle/abs(angle);
		glm::vec3 tmp;
		if (rotation_axis == 'x') {
			tmp = cube_y_axis;
			cube_y_axis = -sign * cube_z_axis;
			cube_z_axis = sign * tmp;
		} else if (rotation_axis == 'y') {
			tmp = cube_z_axis;
			cube_z_axis = -sign * cube_x_axis;
			cube_x_axis = sign * tmp;
		} else {
			tmp = cube_x_axis;
			cube_x_axis = -sign * cube_y_axis;
			cube_y_axis = sign * tmp;	
		}
	}
	
	if (prev_angle > 90) {
		rotate_cube(prev_angle - 90, axis);
	} else if (prev_angle < -90) {
		rotate_cube(prev_angle + 90, axis);
	}
}

vector<vector<vector<Shape*>>> Game::make_cube()
{
	vector<Shape*> line(RUBIKS_CUBE_SIZE);
	vector<vector<Shape*>> face(RUBIKS_CUBE_SIZE, line);
	vector<vector<vector<Shape*>>> cube(RUBIKS_CUBE_SIZE, face);

	return cube;
}

bool Game::rotate_face(float angle, glm::vec3 axis, int index, map<Face, float>& angles_rotated)
{
	float prev_angle = angle;
	if (angle > 90)
		angle = 90;
	else if (angle < -90)
		angle = -90;
	
	char rotation_axis = 'z';
	if (glm::all(glm::equal(axis, cube_x_axis)) || glm::all(glm::equal(axis, glm::vec3(1, 0, 0)))) {
		rotation_axis = 'x';
	} else if (glm::all(glm::equal(axis,cube_y_axis)) || glm::all(glm::equal(axis, glm::vec3(0, 1, 0)))) {
		rotation_axis = 'y';
	}
	
	for (int j = 0; j < RUBIKS_CUBE_SIZE; j++) {
		for (int i = 0; i < RUBIKS_CUBE_SIZE; i++) {
			Shape* cube = rubicks_cube[i][j][index];
			if (rotation_axis == 'x')
				cube = rubicks_cube[index][i][j];
			else if (rotation_axis == 'y')
				cube = rubicks_cube[i][index][j];
						
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

	Face face = make_tuple(rotation_axis, index);
	bool rotated_data_strucrture = angles_rotated[face] >= 45 || angles_rotated[face] <= -45;
	if (rotated_data_strucrture) {
		vector<vector<vector<Shape*>>> new_rubicks_cube = rubicks_cube;
		rotate_data_structure(rotation_axis, index, new_rubicks_cube, angle);
		rubicks_cube = new_rubicks_cube;
		angles_rotated[face] -= (angle/abs(angle)) * 90;
	}

	if (prev_angle > 90) {
		return rotate_face(prev_angle - 90, axis, index, angles_rotated);
	} else if (prev_angle < -90) {
		return rotate_face(prev_angle + 90, axis, index, angles_rotated);
	}
	
	return rotated_data_strucrture;
}

void Game::rotate_data_structure(char axis, int index, vector<vector<vector<Shape*>>>& new_rubicks_cube, float angle)
{
	//TODO: rotation over 90 degrees doesnt update the data structure properly (only does half a turn of the data structure)
	for (int j = 0; j < RUBIKS_CUBE_SIZE; j++) {
		for (int i = 0; i < RUBIKS_CUBE_SIZE; i++) {
			if (axis == 'x') {
				Shape* cube = rubicks_cube[index][i][j];
				if (angle < 0)
					new_rubicks_cube[index][RUBIKS_CUBE_SIZE-1 - j][i] = cube;
				else
					new_rubicks_cube[index][j][RUBIKS_CUBE_SIZE-1 - i] = cube;
			} else if (axis == 'y') {
				Shape* cube = rubicks_cube[i][index][j];
				if (angle > 0)
					new_rubicks_cube[RUBIKS_CUBE_SIZE-1 - j][index][i] = cube;
				else 
					new_rubicks_cube[j][index][RUBIKS_CUBE_SIZE-1 - i] = cube;
			} else {
				Shape* cube = rubicks_cube[i][j][index];
				if (angle > 0)
					new_rubicks_cube[RUBIKS_CUBE_SIZE-1 - j][i][index] = cube;
				else
					new_rubicks_cube[j][RUBIKS_CUBE_SIZE-1 - i][index] = cube;
			} 
		}
	}
}

Game::~Game(void)
{
}
