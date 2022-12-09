#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "my_shape.h"
#include "light.h"


//constants:
glm::vec4 SPECULARVALUE(0.7, 0.7, 0.7, 0.f); 

//global variables:
std::vector<MyShape> my_shapes; 
std::vector<Light> lights; 
glm::vec3 camera;
glm::vec4 ambient_light;
int screen_size;
int width;
int height;
float pixel_height;
float pixel_width;

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

	parse_scene(scene_path);
	
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width/4; j++) {
			
			glm::vec3 pixel_coordinates = get_pixel_coordinates(i, j);
			glm::vec3 ray_direction = glm::normalize(pixel_coordinates - camera);
			glm::vec4 color = send_ray(camera, ray_direction, -1);
			for (int k = 0; k < 4; k++) {
				data[i*width + j*4 + k] = color[k];
			}
		}
	}

}

glm::vec3 Game::get_pixel_coordinates(int i, int j)
{
	float top_left_corner_of_pixel_y = ((i / height) * 2 - 1) * -1;
	float top_left_corner_of_pixel_x = (j / (width/4)) * 2 - 1;

	float center_of_pixel_y = top_left_corner_of_pixel_y - pixel_height/2;
	float center_of_pixel_x = top_left_corner_of_pixel_x + pixel_width/2;

	glm::vec3 pixel_coordinates(center_of_pixel_x, center_of_pixel_y, 0.f);
	return pixel_coordinates;

}

glm::vec4 Game::send_ray(glm::vec3 origin, glm::vec3 direction, int intersecting_shape_index)
{
	glm::vec3 intersection_point(INFINITY, INFINITY, INFINITY);
	int intersecting_shape_index = -1;
	for (int i = 0; i < my_shapes.size(); i++) {
		if (i != intersecting_shape_index) {
			glm::vec3 new_intersection_point = check_shape_intersection(i, origin, direction);
			if (intersection_point[2] > new_intersection_point[2]) {
				intersection_point = new_intersection_point;
				intersecting_shape_index = i;
			}
		}
	}

	MyShape shape = my_shapes[intersecting_shape_index];
	glm::vec4 color = shape.color * ambient_light;
	if (intersection_point[0] != INFINITY) {
		for (int i = 0; i < lights.size(); i++) {
			color += diffuse(intersection_point, intersecting_shape_index, i);
			color += specular(origin, intersection_point, intersecting_shape_index, i);
		}
	}

	return color;
}

void Game::parse_scene(std::string& scene_path)
{
	//TODO:implement
	//parse the scene.txt file
}

glm::vec3 Game::check_shape_intersection(int shape_index, glm::vec3 origin, glm::vec3 direction)
{
	MyShape shape = my_shapes[shape_index];
	glm::vec3 intersection_point(INFINITY, INFINITY, INFINITY);
	if (shape.coordinates[3] > 0) {
		//shape is a sphere:
		glm::vec3 O = glm::vec3(shape.coordinates);
		glm::vec3 L = O - origin;
		float r = shape.coordinates[3];
		float tm = glm::dot(L, direction);
		float d2 = pow(glm::length(L), 2) - tm;

		if (d2 <= pow(r, 2)) {
			float th = sqrt(pow(r, 2) - d2);
			float t = tm - th;
			intersection_point = origin + t * direction;
		}
		return intersection_point;
	} else {
		//shape is a plane:
		glm::vec3 N = glm::vec3(shape.coordinates);
		float d = shape.coordinates[3];
		float NdotV = glm::dot(N, direction);

		if (NdotV != 0) {
			float t = -1 * (glm::dot(N, origin) + d)/NdotV;
			intersection_point = origin + t * direction;
		} 
		return intersection_point;
	}

}

bool Game::check_light_intersection(int light_index, int intersecting_shape_index, glm::vec3 intersection_point)
{
	Light light = lights[light_index];

	bool is_directional_or_intersection_point_is_in_the_spotlight = false;
	if (light.cos_of_angle != INFINITY) {
		glm::vec3 direction_to_light = glm::normalize(light.location - intersection_point);
		if (glm::dot(direction_to_light, light.direction) > light.cos_of_angle) {
			is_directional_or_intersection_point_is_in_the_spotlight = true;
		}
	} else {
		is_directional_or_intersection_point_is_in_the_spotlight = true;
	}

	if (is_directional_or_intersection_point_is_in_the_spotlight) {
		glm::vec3 light_intersection_point(INFINITY, INFINITY, INFINITY);
		for (int i = 0; i < my_shapes.size(); i++) {
			if (i != intersecting_shape_index) {
				light_intersection_point = check_shape_intersection(i, intersection_point, light.direction);
				if (light_intersection_point[0] != INFINITY) {
					return false;
				}
			}
		}
		return true;	
	}

	// if (light.cos_of_angle == INFINITY) {
	// 	//light is a directional light
	// 	glm::vec3 light_intersection_point(INFINITY, INFINITY, INFINITY);
	// 	for (int i = 0; i < my_shapes.size(); i++) {
	// 		if (i != intersecting_shape_index) {
	// 			light_intersection_point = check_shape_intersection(i, intersection_point, light.direction);
	// 			if (light_intersection_point[0] != INFINITY) {
	// 				return 0.f;
	// 			}
	// 		}
	// 	}
	// 	return 1.f;		

	// } else {
	// 	//light is a spotlight
	// 	glm::vec3 direction_to_light = light.location - intersection_point;
	// 	glm::normalize(direction_to_light);

	// 	if (glm::dot(direction_to_light, light.direction) > light.cos_of_angle) {
	// 		//it means our point is inside the beam of light

	// 		glm::vec3 light_intersection_point(INFINITY, INFINITY, INFINITY);
	// 		for (int i = 0; i < my_shapes.size(); i++) {
	// 			if (i != intersecting_shape_index) {
	// 				light_intersection_point = check_shape_intersection(i, intersection_point, light.direction);
	// 				if (light_intersection_point[0] != INFINITY) {
	// 					return 0.f;
	// 				}
	// 			}
	// 		}
	// 		return 1.f;
	// 	}

	// 	return 0.f;

	// }
}

glm::vec4 Game::diffuse(glm::vec3 intersection_point, int shape_index, int light_index)
{
	//TODO:implement
	//calculates the diffuse part for every light source
	//this function will call check_light_intersection for every light source

	glm::vec4 diffuse_color(0.f, 0.f, 0.f, 0.f);
	
	if (check_light_intersection(light_index, shape_index, intersection_point)) {
		MyShape shape = my_shapes[shape_index];
		glm::vec3 N;
		if (shape.coordinates[3] > 0) {
			//shape is a sphere
			glm::vec3 O = glm::vec3(shape.coordinates);
			N = glm::normalize(intersection_point - O);
		} else {
			//shape is a plane
			N = glm::vec3(shape.coordinates);
		}

		Light light = lights[light_index];
		glm::vec3 Li;
		if (light.cos_of_angle != INFINITY) {
			//light is a spotlight
			Li = glm::normalize(light.location - intersection_point);
		} else {
			//light is directional
			Li = light.direction;
		}
		diffuse_color += shape.color * glm::dot(N, Li) * light.intensity;
	}

	return diffuse_color;
}

glm::vec4 Game::specular(glm::vec3 origin, glm::vec3 intersection_point, int shape_index, int light_index)
{
	//TODO:implement
	//calculates the specular part for every light source
	//this function will call check_light_intersection for every light source

	MyShape shape = my_shapes[shape_index];
	glm::vec3 N;
	glm::vec4 specular_color(0.f, 0.f, 0.f, 0.f);
	if (shape.coordinates[3] > 0) {
		//shape is a sphere
		glm::vec3 O = glm::vec3(shape.coordinates);
		N = glm::normalize(intersection_point - O);
	} else {
		//shape is a plane
		N = glm::vec3(shape.coordinates);
	}

	if (check_light_intersection(light_index, shape_index, intersection_point)) {
		Light light = lights[light_index];
		glm::vec3 direction_from_light;
		if (light.cos_of_angle != INFINITY) {
			//light is a spotlight
			direction_from_light = glm::normalize(intersection_point - light.location);
		} else {
			//light is directional
			direction_from_light = light.direction;
		}

		glm::vec3 V = glm::normalize(intersection_point - origin);
		glm::vec3 Ri = glm::reflect(direction_from_light, N);
		
		specular_color +=  SPECULARVALUE * pow(glm::dot(V, Ri), shape.shininess) * light.intensity;
	}

	return specular_color;
}

