#include <tuple>
#include <string>

class Light
{
private:
	std::tuple<float,float,float> direction;
	std::tuple<float,float,float> location; //TODO: what to do if it's directional? Can it be None?

public:
	Light(std::string line);
	//TODO: rule of three?
	~Light(void);
};

