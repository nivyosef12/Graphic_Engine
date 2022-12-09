#include <tuple>
#include <string>

class Light
{
public:
	glm::vec3 direction;
	glm::vec4 intensity;
	glm::vec3 location; //if it's directional, location is (NAN,NAN,NAN)


	Light(glm::vec4 direction);
	void set_intensity(glm::vec4 intensity);
	void set_location(glm::vec4 location);
	~Light(void);
};

