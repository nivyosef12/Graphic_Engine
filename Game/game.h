#pragma once
#include "scene.h"
#include "display.h"
#include <map>

using namespace std;

typedef tuple<char, int> Face;

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
    void shuffle();
	void change_cube_axes(glm::vec3 axis);
    void rotate_cube(float angle, glm::vec3 axis);
    vector<vector<vector<Shape*>>> make_cube();
	bool rotate_face(float angle, glm::vec3 axis, int index, map<Face, float>& angles_rotated);
	void rotate_data_structure(char axis, int index, vector<vector<vector<Shape*>>>& new_rubicks_cube, float angle);
	~Game(void);

private:
	vector<vector<vector<Shape*>>> rubicks_cube;
};

