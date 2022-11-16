#pragma once
#include "scene.h"

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
	~Game(void);
private:
	void edge_detection(int width, int height, unsigned char* data);
	void halftone_pixel(unsigned char* data, unsigned char* new_data, int pixel_num, int width, std::vector<std::vector<unsigned char>> halftone_patterns);
	void halftone(unsigned char* data, unsigned char* new_data, int width, int height);
	void floyd_steinberg_pixel(std::vector<std::vector<float>>& new_data_float_values, int row_num, int column_num, int width, int height, std::vector<float> colors);
	void floyd_steinberg(unsigned char* data, unsigned char* new_data, int width, int height);
	void print_matrix(unsigned char* data, int width, int height);

};

