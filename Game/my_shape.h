#include <tuple>
#include <string>

class MyShape
{
public:
	glm::vec4 coordinates;
	glm::vec4 color;
	char o_r_t;
	float shininess;

	MyShape(char type, glm::vec4 coordinates);
	void set_color(glm::vec4 color);
	//TODO: rule of three?
	~MyShape(void);
};

