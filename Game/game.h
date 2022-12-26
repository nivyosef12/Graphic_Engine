#pragma once
#include "scene.h"
#include "display.h"

using namespace std;

class Game : public Scene
{
public:
	
	Game();
	Game(float angle,float relationWH,float near, float far);
	void Init();
	void Update(const glm::mat4 &MVP,const glm::mat4 &Model,const int  shaderIndx);
	void ControlPointUpdate();
	void WhenRotate();
	void WhenTranslate();
	void Motion();
	static void my_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void rotate_cube(float angle, glm::vec3 axis);
	vector<vector<vector<Shape*>>> make_cube();
	~Game(void);

private:
	vector<vector<vector<Shape*>>> rubicks_cube;
};

