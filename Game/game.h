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
	void halftone_pixel(unsigned char* data, unsigned char* new_data, int pixel_num, std::vector<std::vector<unsigned char>>& halftone_patterns);
	void halftone(unsigned char* data, unsigned char* new_data);
  
	void floyd_steinberg_pixel(std::vector<std::vector<float>>& new_data_float_values, int row_num, int column_num, std::vector<float>& colors);
	void floyd_steinberg(unsigned char* data, unsigned char* new_data);
  
	void print_matrix(unsigned char* data, const std::string file_name, int type_of_image);
 
	void edge_detection(unsigned char* data, unsigned char* new_data);
	void smoothing(unsigned char* data, unsigned char* new_data);
	void derivative(unsigned char* data, unsigned char* new_data, float* pixel_theta);
	void convolution(unsigned char* data, unsigned char* new_data, std::vector<std::vector<float>>& kernel);
	void non_max_suppression(unsigned char* data, unsigned char* new_data, float* pixel_theta);
	void hysteresis(unsigned char* data, unsigned char* new_data);
	int check_neighbor(int x, int y, int z, int up_down, int left_right);
	void thresholding_pixel(unsigned char* data, unsigned char* new_data, std::vector<std::vector<int>>& neighbors_ref, int index);

};

