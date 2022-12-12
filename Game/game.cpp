#include "game.h"
#include <iostream>
#include <fstream>
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
int DATASIZE;
float WIDTH;
float HEIGHT;
int NUMOFCOLORS;
float PIXELHEIGHT;
float PIXELWIDTH;

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
	
	WIDTH = 256.f;
	HEIGHT = 256.f;
	PIXELHEIGHT = 2.f / HEIGHT;
	PIXELWIDTH = 2.f / WIDTH;
	NUMOFCOLORS = 4;
	DATASIZE = WIDTH * HEIGHT * NUMOFCOLORS;
	unsigned char* data = new unsigned char[DATASIZE];

	for (int i = 0; i < DATASIZE; i++)
		data[0] = 0;

	std::string path = "../res/scenes/test_scene.txt";
	ray_tracing(path, data);
	AddTexture(WIDTH, HEIGHT, data);

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

void Game::ray_tracing(std::string& scene_path, unsigned char* data)
{
	parse_scene(scene_path);
	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			
			glm::vec3 pixel_coordinates = get_pixel_coordinates(i, j);
			// printf("pixel_coordinates point: (%f, %f, %f)\n", pixel_coordinates[0], pixel_coordinates[1], pixel_coordinates[2]);
			glm::vec3 ray_direction = glm::normalize(pixel_coordinates - camera);
			glm::vec4 color = send_ray(camera, ray_direction, -1);
			for (int k = 0; k < 4; k++) {
				// maybe WIDTH * 4
				data[i*(int)WIDTH*4 + j*4 + k] = color[k];
			}
		}
	}
	
}

glm::vec3 Game::get_pixel_coordinates(int i, int j)
{
	float top_left_corner_of_pixel_y = ((i / HEIGHT) * 2 - 1) * -1;
	float top_left_corner_of_pixel_x = (j / (WIDTH)) * 2 - 1;

	float center_of_pixel_y = top_left_corner_of_pixel_y - PIXELHEIGHT/2;
	float center_of_pixel_x = top_left_corner_of_pixel_x + PIXELWIDTH/2;

	glm::vec3 pixel_coordinates(center_of_pixel_x, center_of_pixel_y, 0.f);
	return pixel_coordinates;
}

glm::vec4 Game::send_ray(glm::vec3 origin, glm::vec3 direction, int previous_intersecting_shape_index)
{
	glm::vec3 intersection_point(-INFINITY, -INFINITY, -INFINITY);
	int intersecting_shape_index = -1;
	float dist_to_intersection = INFINITY;
	for (int i = 0; i < my_shapes.size(); i++) {
		if (i != previous_intersecting_shape_index) {
			glm::vec3 new_intersection_point = check_shape_intersection(i, origin, direction)[0];

			float new_dist = glm::length(new_intersection_point - origin);
			if (new_dist < dist_to_intersection) {
				intersection_point = new_intersection_point;
				intersecting_shape_index = i;
			}
		}
	}

	glm::vec4 color(0.f, 0.f, 0.f, 0.f);
	//printf("shape index: %f\n", my_shapes[intersecting_shape_index].coordinates[3]);
	if (intersecting_shape_index != -1) {
		//printf("shape index: %i\n", intersecting_shape_index);
		MyShape shape = my_shapes[intersecting_shape_index];
		for (int i = 0; i < lights.size(); i++) {
			// for on shapes and for each i,j calc (check_light_intersection) -> diffuse specular
			if (check_light_intersection(i, intersecting_shape_index, intersection_point)) {
				//color += diffuse(intersection_point, intersecting_shape_index, i);
				color += specular(origin, intersection_point, intersecting_shape_index, i);
			}
		}
		//color /= 1.7;
		color += shape.color * ambient_light;

	}
	// printf("(%f, %f, %f, %f)\n", color[0], color[1], color[2], color[3]);
	if (color[0] == INFINITY)
	{
		printf("?");
	}

	return color*255.f;
}


std::vector<glm::vec3> Game::check_shape_intersection(int shape_index, glm::vec3 origin, glm::vec3 direction)
{
	MyShape shape = my_shapes[shape_index];
	glm::vec3 intersection_point1(-INFINITY, -INFINITY, -INFINITY);
	glm::vec3 intersection_point2(-INFINITY, -INFINITY, -INFINITY);
	std::vector<glm::vec3> intersection_points;
	if (shape.coordinates[3] > 0) {
		//shape is a sphere:
		glm::vec3 O = glm::vec3(shape.coordinates);
		glm::vec3 L = (O - origin);
		float r = shape.coordinates[3];
		float tm = glm::dot(L, direction);
		float d2 = pow(glm::length(L), 2) - pow(tm, 2);

		if (d2 <= pow(r, 2)) {

			float th = sqrt(pow(r, 2) - d2);
			float t1 = tm - th;
			float t2 = tm + th;
			intersection_point1 = origin + t1 * direction;
			intersection_point2 = origin + t2 * direction;

			float dist1 = glm::length(origin - intersection_point1);
			float dist2 = glm::length(origin - intersection_point2);

			if (dist1 < dist2) {
				intersection_points.push_back(intersection_point1);
				intersection_points.push_back(intersection_point2);
			}
			else {
				intersection_points.push_back(intersection_point2);
				intersection_points.push_back(intersection_point1);
			}
		}
		/*else {
			intersection_points.push_back(intersection_point1);
			intersection_points.push_back(intersection_point2);

		}*/
	} else {
		//shape is a plane:
		glm::vec3 N = glm::normalize(glm::vec3(shape.coordinates));
		float d = shape.coordinates[3];
		float NdotV = glm::dot(N, direction);

		if (NdotV != 0) {
			float t = -1 * (glm::dot(N, origin) + d)/NdotV;
			intersection_point1 = origin + t * direction;
		}
		/*intersection_points.push_back(intersection_point1);
		intersection_points.push_back(intersection_point2);*/

	}
	intersection_points.push_back(intersection_point1);
	intersection_points.push_back(intersection_point2);
	return intersection_points;
	//TODO:planes dont cast shadows
}

bool Game::check_light_intersection(int light_index, int intersecting_shape_index, glm::vec3 intersection_point)
{
	Light light = lights[light_index];

	bool is_directional_or_intersection_point_is_in_the_spotlight = false;
	glm::vec3 direction_from_light = glm::normalize(light.direction);
	if (light.cos_of_angle != INFINITY) {
		direction_from_light = glm::normalize(intersection_point - light.location);
		if (glm::dot(direction_from_light, glm::normalize(light.direction)) > light.cos_of_angle) {
			is_directional_or_intersection_point_is_in_the_spotlight = true;
		}
	} else {
		is_directional_or_intersection_point_is_in_the_spotlight = true;
	}

	if (is_directional_or_intersection_point_is_in_the_spotlight) {
		glm::vec3 light_intersection_point(-INFINITY, -INFINITY, -INFINITY);
		for (int i = 0; i < my_shapes.size(); i++) {
			if (i != intersecting_shape_index && my_shapes[i].coordinates[3] > 0) {
				// fursther intersection point
				// test case
				light_intersection_point = check_shape_intersection(i, intersection_point, -direction_from_light)[0];

				if (light_intersection_point[0] != -INFINITY) {

					return false;
				}
			}
			//else if (i == intersecting_shape_index && my_shapes[i].coordinates[3] > 0) {
			//	// fursther intersection point
			//	light_intersection_point = check_shape_intersection(i, intersection_point, -direction_from_light)[1];

			//	if (!glm::equal(light_intersection_point, glm::vec3(0.f, 0.f, 0.f))[0] &&
			//		glm::equal(glm::normalize(light_intersection_point - intersection_point), -direction_from_light)[0])
			//		return false;

			//}
		}
		return true;	
	}

	/*if (is_directional_or_intersection_point_is_in_the_spotlight) {
		glm::vec3 light_intersection_point(-INFINITY, -INFINITY, -INFINITY);
		for (int i = 0; i < my_shapes.size(); i++) {
			light_intersection_point = check_shape_intersection(i, intersection_point, -direction_from_light);
			if (light_intersection_point[0] != -INFINITY) {
				glm::vec3 direction2 = light_intersection_point - intersection_point;
				if (!glm::equal(direction2, glm::vec3(0.f, 0.f, 0.f))[0] && !glm::equal(glm::normalize(direction2), direction_from_light)[0] && my_shapes[i].coordinates[3] > 0) {
					light_intersection_point = check_shape_intersection(i, intersection_point, direction_from_light);
					if (light_intersection_point[0] != -INFINITY) {
						return false;
					}
				}
			}
		}
		return true;
	}*/
	return false;
}

glm::vec4 Game::diffuse(glm::vec3 intersection_point, int shape_index, int light_index)
{
	glm::vec4 diffuse_color(0.f, 0.f, 0.f, 0.f);
	MyShape shape = my_shapes[shape_index];
	glm::vec3 N = glm::normalize(glm::vec3(shape.coordinates));
	if (shape.coordinates[3] > 0) {
		//shape is a sphere
		glm::vec3 O = glm::vec3(shape.coordinates);
		N = glm::normalize(intersection_point - O);
	}

	Light light = lights[light_index];
	glm::vec3 Li = glm::normalize(-light.direction);
	if (light.cos_of_angle != INFINITY) {
		//light is a spotlight
		Li = glm::normalize(light.location - intersection_point);
	}
	diffuse_color += shape.color * glm::dot(N, Li) * light.intensity;

	return diffuse_color;
}

glm::vec4 Game::specular(glm::vec3 origin, glm::vec3 intersection_point, int shape_index, int light_index)
{
	MyShape shape = my_shapes[shape_index];
	glm::vec3 N = glm::normalize(glm::vec3(shape.coordinates));
	glm::vec4 specular_color(0.f, 0.f, 0.f, 0.f);
	if (shape.coordinates[3] > 0) {
		//shape is a sphere
		glm::vec3 O = glm::vec3(shape.coordinates);
		N = glm::normalize(intersection_point - O);
	}

	Light light = lights[light_index];
	glm::vec3 direction_from_light = light.direction;
	if (light.cos_of_angle != INFINITY) {
		//light is a spotlight
		direction_from_light = glm::normalize(light.location - intersection_point);
	}

	glm::vec3 V = glm::normalize(origin - intersection_point);
	glm::vec3 Ri = glm::reflect(direction_from_light, N);
	
	specular_color +=  SPECULARVALUE * pow(glm::dot(V, Ri), shape.shininess) * light.intensity;

	return specular_color;
}

int Game::split(const std::string& txt, std::vector<std::string>& strs, char delimeter) {
	size_t pos = txt.find(delimeter);
	size_t initialPos = 0;
	strs.clear();

	// Decompose statement
	while (pos != std::string::npos) {
		strs.push_back(txt.substr(initialPos, pos - initialPos));
		initialPos = pos + 1;

		pos = txt.find(delimeter, initialPos);
	}

	// Add the last one
	strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

	return strs.size();
}


void Game::parse_scene(std::string& scene_path)
{
	std::string line;
	std::ifstream myfile;
	myfile.open(scene_path);

	std::vector<std::string> splitted;
	int colorInstruction = 0;
	int intensityInstruction = 0;
	int spotlightIndex = 0;
	std::vector<int> spotlightsIndexes;

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			split(line, splitted, ' ');

			if (splitted[0] == "e") {
				camera = glm::vec3(std::stof(splitted[1]), std::stof(splitted[2]), std::stof(splitted[3]));
			}

			else if (splitted[0] == "a") {
				ambient_light = glm::vec4(std::stof(splitted[1]), std::stof(splitted[2]),
					std::stof(splitted[3]), std::stof(splitted[4]));
			}

			else if (splitted[0] == "o" || splitted[0] == "t" || splitted[0] == "r") {

				MyShape shape = MyShape(splitted[0], glm::vec4(std::stof(splitted[1]), std::stof(splitted[2]),
					std::stof(splitted[3]), std::stof(splitted[4])));
				my_shapes.push_back(shape);

			}

			else if (splitted[0] == "c") {
				my_shapes[colorInstruction].set_color_and_shininess(glm::vec4(std::stof(splitted[1]), std::stof(splitted[2]),
					std::stof(splitted[3]), std::stof(splitted[4])));
				colorInstruction += 1;

			}

			else if (splitted[0] == "d") {
				Light light = Light(glm::vec4(std::stof(splitted[1]), std::stof(splitted[2]),
					std::stof(splitted[3]), std::stof(splitted[4])));
				if (splitted[4] == "1.0")
					spotlightsIndexes.push_back(spotlightIndex);

				spotlightIndex += 1;
				lights.push_back(light);
			}

			else if (splitted[0] == "p") {
				lights[spotlightsIndexes[0]].set_location(glm::vec4(std::stof(splitted[1]), std::stof(splitted[2]),
					std::stof(splitted[3]), std::stof(splitted[4])));
				spotlightsIndexes.erase(spotlightsIndexes.begin());

			}

			else if (splitted[0] == "i") {
				lights[intensityInstruction].set_intensity(glm::vec4(std::stof(splitted[1]), std::stof(splitted[2]),
					std::stof(splitted[3]), std::stof(splitted[4])));
				intensityInstruction += 1;
			}
		}

		myfile.close();
	}

	else {
		printf("Unable to open file\n");
	}
}
