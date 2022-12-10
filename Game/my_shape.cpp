#include "my_shape.h"

MyShape::MyShape(char type, glm::vec4 coordinates)
{
	this->coordinates = coordinates;
	this->o_r_t = type;
}

void MyShape::set_color_and_shininess(glm::vec4 color)
{
	glm::vec3 rgb = glm::vec3(color);
	this->color = glm::vec4(rgb, 0.f);
	this->shininess = color[3];
}

MyShape::~MyShape(void)
{
}
