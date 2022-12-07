#include <tuple>
#include <string>

class MyShape
{
private:
	std::tuple<float,float,float,float> coordinates;
	std::tuple<float,float,float,float> color;
	char o_r_t;

public:
	MyShape(std::string line);
	//TODO: rule of three?
	~MyShape(void);
};

