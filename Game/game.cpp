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

	std::string path = "../res/scenes/scene3.txt";
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
			glm::vec3 ray_direction = glm::normalize(pixel_coordinates - camera);
			glm::vec4 color = send_ray(camera, ray_direction, -1);

			for (int k = 0; k < 4; k++) {
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
				dist_to_intersection = new_dist;
			}
		}
	}

	glm::vec4 color(0.f, 0.f, 0.f, 0.f);
	if (intersecting_shape_index != -1) {
		MyShape shape = my_shapes[intersecting_shape_index];

		for (int i = 0; i < lights.size(); i++) {
			if (check_light_intersection(i, intersecting_shape_index, intersection_point)) {
				color += diffuse(origin, intersection_point, intersecting_shape_index, i);
				color += specular(origin, intersection_point, intersecting_shape_index, i);
			}
		}

		color /= (float)lights.size();
		color += shape.color * ambient_light;
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

			return intersection_points;
		}
	} else {
		//shape is a plane:
		//* -------------- 4 --------------
		glm::vec3 p_n = glm::normalize(glm::vec3(shape.coordinates));
		if (glm::dot(p_n, direction) < 0)
			p_n = p_n * -1.f;
		float p_n_dot_r_d = glm::dot(p_n, direction);
		// if p_n_dot_r_d >= 0 stop?
		if (p_n_dot_r_d > 0) {
			float v0 = -((glm::dot(p_n, origin)) + shape.coordinates[3]);
			float t = v0 / p_n_dot_r_d;
			if (t >= 0)
				intersection_point1 = origin + t * direction;
		}

		/* -------------- 1 --------------
		glm::vec3 N = glm::normalize(glm::vec3(shape.coordinates));
		if (glm::dot(N, direction) < 0)
			N = N * 1.f;
		float d = shape.coordinates[3];
		float NdotV = glm::dot(N, direction);

		if (NdotV > 0) {
			float t = -1 * (glm::dot(N, origin) + d)/NdotV;
			intersection_point1 = origin + t * direction;
		}*/

		/* -------------- 2 --------------
		glm::vec3 N = glm::normalize(glm::vec3(shape.coordinates));
		float Vd = glm::dot(N, direction);
		glm::vec3 dir2 = direction;
		if (Vd > 0)
			dir2 *= -1.f;
		float V0 = -(Vd + shape.coordinates[3]);
		float t = V0 / Vd;
		if (t > 0) {
			intersection_point1 = origin + t * dir2;
		}*/

		/* -------------- 3 --------------
		glm::vec3 N = glm::normalize(glm::vec3(shape.coordinates));
		if (glm::dot(N, direction) < 0)
			N = N * -1.f;
		float t = -(glm::dot(N, origin) + shape.coordinates[3]) / (glm::dot(N, direction));
		if (t >= 0) {
			intersection_point1 = origin + t * direction;
			//Does the ray intersect the plane inside or outside?
			glm::vec3 planeToRay = origin - intersection_point1;
			float dot_ = glm::dot(planeToRay, N);
			if (dot_ > 0)
				intersection_point1 = glm::vec3(-INFINITY, -INFINITY, -INFINITY);
		}*/
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

	return false;
}

glm::vec4 Game::diffuse(glm::vec3 origin, glm::vec3 intersection_point, int shape_index, int light_index)
{
	glm::vec4 diffuse_color(0.f, 0.f, 0.f, 0.f);
	MyShape shape = my_shapes[shape_index];
	glm::vec3 N = glm::normalize(glm::vec3(shape.coordinates));
	if (shape.coordinates[3] > 0) {
		//shape is a sphere
		glm::vec3 O = glm::vec3(shape.coordinates);
		N = glm::normalize(intersection_point - O);
	}
	else {
		if (glm::dot(N, (intersection_point - origin)) > 0)
			N = N * -1.f;
	}

	Light light = lights[light_index];
	glm::vec3 Li = glm::normalize(-light.direction);
	if (light.cos_of_angle != INFINITY) {
		//light is a spotlight
		Li = glm::normalize(light.location - intersection_point);
	}
	//diffuse_color += shape.color * std::max(0.f, glm::dot(N, Li)) * light.intensity;
	glm::vec4 shape_color = shape.color;
	if (shape.coordinates[3] < 0) {
		glm::vec2 x_y = shape.get_x_y(intersection_point);
		if ((int)(3 * x_y[0]) % 2 == (int)(3 * x_y[1]) % 2)
			shape_color *= 0.5;
	}
	diffuse_color += shape_color * std::max(0.f, glm::dot(N, Li)) * light.intensity;

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
	else {
		if (glm::dot(N, (intersection_point - origin)) > 0)
			N = N * -1.f;
	}

	Light light = lights[light_index];
	glm::vec3 direction_from_light = light.direction;
	if (light.cos_of_angle != INFINITY) {
		//light is a spotlight
		direction_from_light = glm::normalize(intersection_point - light.location);
	}

	glm::vec3 V = glm::normalize(origin - intersection_point);
	glm::vec3 Ri = glm::normalize(glm::reflect(direction_from_light, N));
	
	//specular_color +=  SPECULARVALUE * pow(glm::dot(V, Ri), shape.shininess) * light.intensity;
	specular_color += SPECULARVALUE * pow(std::max(0.f, glm::dot(V, Ri)), shape.shininess) * light.intensity;


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
