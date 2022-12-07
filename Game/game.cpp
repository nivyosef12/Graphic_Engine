#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>


std::vector<float> shapes; //TODO:not really float, will be determined later on
std::vector<float> lights; //TODO:not really float, will be determined later on
std::tuple<float,float,float,float> camera;
//TODO: we may need to create a new classes MyShape and Light to hold all the information about shapes and lights

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

	AddShape(Plane,-1,TRIANGLES);
	
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

void Game::ray_tracing(std::string& scene_path, int width, int height, unsigned char* data)
{
	//TODO:implement
	//main function, will be called from Init
}

void Game::parse_scene(std::string& scene_path)
{
	//TODO:implement
	//parse the scene.txt file
}

glm::vec3 Game::check_shape_intersection(glm::vec4 shape, glm::vec3 ray_origin, glm::vec3 ray_direction)
{
	//TODO:implement
	//check if ray intersects with shape (sphere or plane) and return the coordinates of the
	//closest intersection or None(?) if there is none (maybe nedds to return more things)
}

bool Game::check_light_intersection(glm::vec4 light, glm::vec3 intersection_point)
{
	//TODO:implement
	//check if the intersection_point between the ray and an object meets light (directional or spotlight),
	//return True or False (maybe nedds to return more things)
}

glm::vec4 Game::diffuse(glm::vec3 intersection_point, glm::vec3 normal)
{
	//TODO:implement
	//calculates the diffuse part for every light source
	//this function will call check_light_intersection for every light source
}

glm::vec4 Game::specular(glm::vec3 intersection_point, glm::vec3 normal)
{
	//TODO:implement
	//calculates the specular part for every light source
	//this function will call check_light_intersection for every light source
}

