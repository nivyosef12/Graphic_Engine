#include "light.h"
#include <glm/gtc/matrix_transform.hpp>

Light::Light(glm::vec4 direction)
{
	this->direction = direction;
}

void Light::set_intensity(glm::vec4 intensity)
{
	this->intensity = intensity;
}

void Light::set_location(glm::vec4 location)
{
	this->location = location;
}

Light::~Light(void)
{
}
