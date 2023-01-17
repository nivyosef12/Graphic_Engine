#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Bezier1D.h"
#include <thread>
#include <chrono>

//params:
int seg = 0;
float t = 0;
float t_increment = 0.0001;
int seg_increment = 1;
Bezier1D* b;
std::vector<Shape*> control_points;
int selected_control_point = 0;
bool isSelected = false;

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

	seg = 0;
	t = 0;
	t_increment = 0.1;
	seg_increment = 1;
	selected_control_point = 0;

	// TODO what is the diff?
	int segNum = 3;
	int res = 3;
	Bezier1D *bezier = new Bezier1D(segNum, res, 1);
	b = bezier;
	float octahedron_scale = 0.25;
	AddShape(Octahedron, -1, TRIANGLES);
	shapes[0]->MyScale(glm::vec3(2*octahedron_scale, 2*octahedron_scale, 2*octahedron_scale));
	control_points.push_back(shapes.back());
	for(int i = 0; i < segNum; i++){
		for(int j = 1; j < 4; j++){
			glm::vec4 curr_point = bezier->GetControlPoint(i, j);
			AddShapeCopy(0, -1, TRIANGLES);
			shapes.back()->MyTranslate(glm::vec3(curr_point.x, curr_point.y, curr_point.z), 0);
			shapes.back()->MyScale(glm::vec3(octahedron_scale, octahedron_scale, octahedron_scale));
			control_points.push_back(shapes.back());
		}
	}
	shapes.push_back(bezier);
	AddShape(Cube, -1, TRIANGLES);

	
	pickedShape = 0;
	
	SetShapeTex(0,0);
	MoveCamera(0,zTranslate,35);
	MoveCamera(0,xTranslate,9);

	pickedShape = -1;
	
	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::MyKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	Game *scn = (Game*)glfwGetWindowUserPointer(window);
	
	if(action == GLFW_PRESS)
	{
		Shape* cube;
		switch (key)
		{			
			case GLFW_KEY_RIGHT:
				scn->SwitchControlPoint(1);
				break;

			case GLFW_KEY_LEFT:
				scn->SwitchControlPoint(-1);
				break;

			case GLFW_KEY_R:
				scn->MoveCamera(0, xTranslate, -5);
				break;

			case GLFW_KEY_L:
				scn->MoveCamera(0, xTranslate, 5);
				break;

			case GLFW_KEY_U:
				scn->MoveCamera(0, yTranslate, 5);
				break;

			case GLFW_KEY_D:
				scn->MoveCamera(0, yTranslate, -5);
				break;

			case GLFW_KEY_B:
				scn->MoveCamera(0, zTranslate, 5);
				break;

			case GLFW_KEY_F:
				scn->MoveCamera(0, zTranslate, -5);
				break;

			case GLFW_KEY_S:
				scn->ActivateSelection();
				break;

			case GLFW_KEY_SPACE:
				if (scn->isActive) {
					scn->Deactivate();
				} else {
					scn->Activate();
				}
				break;
			case GLFW_KEY_2:

				break;
			
		default:
			break;
		}
	}
}

void Game::MyMouseCallback(GLFWwindow* window,int button, int action, int mods) 
{
	if(action == GLFW_PRESS )
	{
		Game *scn = (Game*)glfwGetWindowUserPointer(window);
		double x2,y2;
		glfwGetCursorPos(window,&x2,&y2);
		scn->Picking((int)x2,(int)y2);
	}
}

void Game::MyCursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	Game *scn = (Game*)glfwGetWindowUserPointer(window);
	scn->UpdatePosition((float)xpos,(float)ypos);

	if (!isSelected) {
		// moving curve
		if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			scn->MouseProccessing(GLFW_MOUSE_BUTTON_RIGHT);
		}

		// rotating curve
		else if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			scn->MouseProccessing(GLFW_MOUSE_BUTTON_LEFT);
		}
	} else {
		Shape* control_point = control_points[selected_control_point];
		if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{				

			// control_point->MyTranslate(glm::vec3(-(scn->xrel)/40.0f,0,0),0);
			// control_point->MyTranslate(glm::vec3(0,(scn->yrel)/40.0f,0),0);
			scn->ControlPointUpdate(-(scn->xrel)/40.0f, (scn->yrel)/40.0f, true);
		}
		// else if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		// {
		// 	control_point->MyRotate(-(scn->xrel)/2.0f,glm::vec3(0,1,0),0);
		// 	control_point->MyRotate(-(scn->yrel)/2.0f,glm::vec3(1,0,0),0);
		// }

		scn->Draw(1,0,BACK,true,false);
		glfwSwapBuffers(window);
	}
}

void Game::MyScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Game *scn = (Game*)glfwGetWindowUserPointer(window);
	scn->MoveCamera(0, zTranslate, yoffset);
	
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

void Game::SwitchControlPoint(int i)
{
	if (isSelected) {
		control_points[selected_control_point]->MyScale(glm::vec3(0.5,0.5,0.5));

		selected_control_point += i;
		if (selected_control_point >= control_points.size() - 1)
			selected_control_point = control_points.size() - 1;
		if (selected_control_point < 0)
			selected_control_point = 0;
		
		control_points[selected_control_point]->MyScale(glm::vec3(2,2,2));
	}
}

void Game::ActivateSelection()
{
	isSelected = !isSelected;
}

void Game::ControlPointUpdate(float dx, float dy, bool preserveC1)
{
	control_points[selected_control_point]->MyTranslate(glm::vec3(dx, dy, 0), 0);
	if (preserveC1 && (selected_control_point - 1) % 3 == 0 && selected_control_point != 1) {
		control_points[selected_control_point - 2]->MyTranslate(glm::vec3(-dx, -dy, 0), 0);
	} else if (preserveC1 && (selected_control_point - 1) % 3 == 1 && selected_control_point != control_points.size() - 2) {
		control_points[selected_control_point + 2]->MyTranslate(glm::vec3(-dx, -dy, 0), 0);
	}
	int segNum = selected_control_point/3;
	int index = selected_control_point % 3;
	if (segNum == control_points.size()/4) {
		segNum -= 1;
		index = 3;
	}
	b->MoveControlPoint(segNum, index, dx, dy, preserveC1);
}

void Game::WhenRotate()
{
}

void Game::WhenTranslate()
{
}

void Game::Motion()
{
	// Bezier1D *b = (Bezier1D*) shapes[0];
	Shape *cube = shapes.back();
	int segNum = b->GetSegmentsNum();
	if(isActive)
	{
		t += t_increment;
		if (t_increment > 0 && t >= 1) {
			t = 0;
			seg += 1;
		} else if (t_increment < 0 && t <= 0) {
			t = 1;
			seg -= 1;
		}
		
		if (seg == segNum) {
			seg -= 1;
			t = 1;
			t_increment *= -1;
		} else if (seg < 0) {
			seg = 0;
			t = 0;
			t_increment *= -1;
		}

		glm::vec4 point = b->GetPointOnCurve(seg, t);

		cube->MyTranslate(glm::vec3(), 1);
		cube->MyTranslate(glm::vec3(point.x, point.y, point.z), 0);

		glm::vec4 deriv = b->GetVelocity(seg, t);
		cube->MyRotate(0, glm::vec3(deriv.x, deriv.y, deriv.z), 2);

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

Game::~Game(void)
{
}
