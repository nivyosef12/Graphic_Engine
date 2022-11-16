#pragma once
#include "scene.h"
#include <functional>

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

	void edge_detection(unsigned char* data, unsigned char* new_data, int image_width, int image_height);
	void smoothing(unsigned char* data, unsigned char* new_data, int image_width, int image_height);
	void derivative(unsigned char* data, unsigned char* new_data, float* pixel_theta, int image_width, int image_height);
	void convolution(unsigned char* data, unsigned char* new_data, std::vector<std::vector<float>>& kernel, int data_size, int image_width, int image_height);
	void non_max_suppression(unsigned char* data, unsigned char* new_data, float* pixel_theta, int image_width, int image_height);
	void hysteresis(unsigned char* data, unsigned char* new_data, int image_width, int image_height, double low_threshold, double high_threshold);
	int check_neighbor(int x, int y, int z, int up_down, int left_right, int width, int height);
	void thresholding_pixel(unsigned char* data, unsigned char* new_data, std::vector<std::vector<int>>& neighbors_ref, int index, int width, int height, double low_threshold, double high_threshold);


	void halftone_pixel(unsigned char* data, unsigned char* new_data, int pixel_num, int width, std::vector<std::vector<unsigned char>> halftone_options);
	void halftone(int* width, int* height, unsigned char* data);

	void floyd_steinberg(int width, int height, unsigned char* data);

};

