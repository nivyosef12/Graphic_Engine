#include <tuple>
#include <string>

class Light
{
private:
	glm::vec4 direction;
	glm::vec4 intensity;
	glm::vec4 location; //TODO: what to do if it's directional? Can it be None?

public:
	Light(glm::vec4 direction);
	void set_intensity(glm::vec4 intensity);
	void set_location(glm::vec4 location);
	//TODO: rule of three?
	~Light(void);
};

