#include <tuple>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat3x3.hpp>

class MyShape
{
public:
	glm::vec4 coordinates;
	glm::vec4 color;
	char o_r_t;
	float shininess;
	glm::mat3 x_y_transform;

	MyShape(char type, glm::vec4 coordinates);
	void set_color_and_shininess(glm::vec4 color);
	glm::vec2 get_x_y(glm::vec3 coordinates);
	~MyShape(void);
};

