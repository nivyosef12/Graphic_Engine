#include "my_shape.h"

MyShape::MyShape(std::string type, glm::vec4 coordinates)
{
	this->coordinates = coordinates;
	this->o_r_t = type;
	if (coordinates[3] < 0) {
		float a = coordinates[0];
		float b = coordinates[1];
		float c = coordinates[2];
		float d = coordinates[3];

		float sqrta2b2c2 = sqrt(a*a + b*b + c*c);
		float cos_theta = c / sqrta2b2c2;
		float sin_theta = sqrt(a*a + b*b) / sqrta2b2c2;
		float u1 = b / sqrta2b2c2;
		float u2 = -a / sqrta2b2c2;

		x_y_transform = glm::mat3(
			cos_theta+u1*u1*(1-cos_theta), u1*u2*(1-cos_theta), u2*sin_theta,
			u1*u2*(1-cos_theta), cos_theta+u2*u2*(1-cos_theta), -u1*sin_theta,
			-u2*sin_theta, u1*sin_theta, cos_theta
		);
	}
}

void MyShape::set_color_and_shininess(glm::vec4 color)
{
	glm::vec3 rgb = glm::vec3(color);
	this->color = glm::vec4(rgb, 0.f);
	this->shininess = color[3];
}

glm::vec2 MyShape::get_x_y(glm::vec3 coordinates)
{
	glm::vec3 new_x_y = x_y_transform * coordinates;
	return glm::vec2(new_x_y);
}

MyShape::~MyShape(void)
{
}
