#include <tuple>
#include <string>

class MyShape
{
private:
	glm::vec4 coordinates;
	glm::vec4 color;
	char o_r_t;

public:
	MyShape(char type, glm::vec4 coordinates);
	void set_color(glm::vec4 color);
	//TODO: rule of three?
	~MyShape(void);
};

