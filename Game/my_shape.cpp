#include "my_shape.h"
#include <glm/gtc/matrix_transform.hpp>

MyShape::MyShape(char type, glm::vec4 coordinates)
{
	this->coordinates = coordinates;
	this->o_r_t = type;
}

void MyShape::set_color(glm::vec4 color)
{
	this->color = color;
}


MyShape::~MyShape(void)
{
}
