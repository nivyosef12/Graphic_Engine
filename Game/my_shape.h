#include <tuple>
#include <string>
#include <glm/gtc/matrix_transform.hpp>

class MyShape
{
public:
	glm::vec4 coordinates;
	glm::vec4 color;
	char o_r_t;
	float shininess;

	MyShape(char type, glm::vec4 coordinates);
	void set_color_and_shininess(glm::vec4 color);
	//TODO: rule of three?
	~MyShape(void);
};

