#pragma once
#include "scene.h"
#include <map>

class Game : public Scene
{
public:
	
	Game();
	Game(float angle,float relationWH,float near, float far);
	void Init();
	void Update(const glm::mat4 &MVP,const glm::mat4 &Model,const int  shaderIndx);
	void ControlPointUpdate();
	void WhenRotate();
	void WhenTranslate();
	void Motion();
	~Game(void);

private:
	void ray_tracing(std::string &scene_path, int width, int height, unsigned char *data);
    glm::vec3 get_pixel_coordinates(int i, int j);
    glm::vec4 send_ray_from_pixel(glm::vec3 pixel_coordinates);
    void parse_scene(std::string &scene_path);
    glm::vec3 check_shape_intersection(MyShape shape, glm::vec3 ray);
    bool check_light_intersection(Light light, glm::vec3 intersection_point);
	glm::vec4 diffuse(glm::vec3 intersection_point, glm::vec3 normal);
	glm::vec4 specular(glm::vec3 intersection_point, glm::vec3 normal);

};

