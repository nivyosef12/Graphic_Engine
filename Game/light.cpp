#include "light.h"
#include <glm/gtc/matrix_transform.hpp>

Light::Light(glm::vec4 direction)
{
	this->direction = glm::vec3(direction);
	this->location = glm::vec3(NAN, NAN, NAN);
}

void Light::set_intensity(glm::vec4 intensity)
{
	this->intensity = intensity;
}

void Light::set_location(glm::vec4 location)
{
	this->location = glm::vec3(location);
}

Light::~Light(void)
{
}
