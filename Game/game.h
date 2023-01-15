#pragma once
#include "scene.h"
#include "display.h"

class Game : public Scene
{
public:
	
	Game();
	Game(float angle,float relationWH,float near, float far);
	void Init();
    static void MyKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void MyMouseCallback(GLFWwindow *window, int button, int action, int mods);
    static void MyCursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void MyScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    void Update(const glm::mat4 &MVP, const glm::mat4 &Model, const int shaderIndx);
    void ControlPointUpdate();
	void WhenRotate();
	void WhenTranslate();
	void Motion();
	~Game(void);
};

