// TODO list
// 1. global vars
// 2. print matrix
// 3. sobel

#define _USE_MATH_DEFINES

#include "game.h"
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <cmath>
#include <map>

static void printMat(const glm::mat4 mat)
{
	std::cout<<" matrix:"<<std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout<< mat[j][i]<<" ";
		std::cout<<std::endl;
	}
}

Game::Game() : Scene()
{
}

Game::Game(float angle ,float relationWH, float near1, float far1) : Scene(angle,relationWH,near1,far1)
{ 	
}

void Game::Init()
{		
	AddShader("../res/shaders/pickingShader");	
	AddShader("../res/shaders/basicShader");

	AddShape(Plane, -1, TRIANGLES);

	//add grayscale texture
	std::string fileName = "../res/textures/5.jpg";
	int width, height, numComponents;
	unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4); //extract data from the image
	if (data == NULL) {
		std::cerr << "Unable to load texture: " << fileName << std::endl;
		exit(1);
	}
	AddTexture(fileName, false);

	//add texture for edge detection
	unsigned char* edge_detection_data = new unsigned char[width * height * 4];
	edge_detection(data, edge_detection_data, width, height);
	AddTexture(width, height, edge_detection_data);
	delete[] edge_detection_data;

	//add texture for halftone
	int width2 = width * 2;
	int height2 = height * 2;
	unsigned char* halftone_data = new unsigned char[width2 * height2 * 4];
	halftone(data, halftone_data, width2, height2);
	AddTexture(width2, height2, halftone_data);
	delete[] halftone_data;

	//add texture for floyd-steinberg
	unsigned char* floyd_steinberg_data = new unsigned char[width * height * 4];
	floyd_steinberg(data, floyd_steinberg_data, width, height);
	AddTexture(width, height, floyd_steinberg_data);
	delete[] floyd_steinberg_data;
	
	MoveCamera(0,zTranslate,1.5);
	pickedShape = -1;
	
	stbi_image_free(data);

	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::edge_detection(unsigned char* data, unsigned char* new_data, int width, int height)
{
	unsigned char* smoothed_data = new unsigned char[4 * width * height]; //TODO: memory leaks!!
	unsigned char* derived_data = new unsigned char[4 * width * height]; //TODO: memory leaks!!
	float* pixel_theta = new float[4 * width * height]; //TODO: memory leaks!!
	unsigned char* non_max_suppression_data = new unsigned char[4 * width * height]; //TODO: memory leaks!!

	smoothing(data, smoothed_data, width, height);

	derivative(smoothed_data, derived_data, pixel_theta, width, height);

	non_max_suppression(derived_data, non_max_suppression_data, pixel_theta, width, height);

	hysteresis(non_max_suppression_data, new_data, width, height, 18, 40);

	delete[] smoothed_data;
	delete[] derived_data;
	delete[] pixel_theta;
	delete[] non_max_suppression_data;
}

void Game::smoothing(unsigned char* data, unsigned char* new_data, int image_width, int image_height) {

	// generate Gaussian filter kernel of size (2k +1)X(2k + 1)
	const int data_size = 4 * image_height * image_width;
	const int k = 2;
	const int kernel_size = (2 * k) + 1;
	const double sigma = 1.175;
	std::vector<std::vector<float>> gaussian(kernel_size, std::vector<float>(kernel_size, 0));

	for (int i = 0; i < kernel_size; i++) {
		for (int j = 0; j < kernel_size; j++) {

			float normal = 1 / (2.0 * M_PI * pow(sigma, 4));
			gaussian[i][j] = normal * exp(-((pow(i - k, 2) + pow(j - k, 2)) / (2.0 * pow(sigma, 2))));

		}
	}
	std::vector<std::vector<float>>& gaussian_ref = gaussian;

	convolution(data, new_data, gaussian_ref, data_size, image_width, image_height);  // smoothing with gaussian filter

}

void Game::derivative(unsigned char* data, unsigned char* new_data, float* pixel_theta, int image_width, int image_height) {

	const int data_size = 4 * image_height * image_width;
	unsigned char* new_data_gx = new unsigned char[data_size]; 
	unsigned char* new_data_gy = new unsigned char[data_size];

	std::vector<std::vector<float>> Gx{
	{1, 0, -1},
	{2, 0, -2},
	{1, 0, -1}
	};
	std::vector<std::vector<float>> Gy{
	{1, 2, 1},
	{0, 0, 0},
	{-1, -2, -1}
	};
	/*
	float param = 1.7;
	std::vector<std::vector<float>> Gx{
	{0, 0, 0},
	{0, param, -param},
	{0, 0, 0}
	};
	std::vector<std::vector<float>> Gy{
	{0, 0, 0},
	{0, param, 0},
	{0, -param, 0}
	};
	*/
	std::vector<std::vector<float>>& Gx_ref = Gx;
	std::vector<std::vector<float>>& Gy_ref = Gy;

	convolution(data, new_data_gx, Gx_ref, data_size, image_width, image_height);
	convolution(data, new_data_gy, Gy_ref, data_size, image_width, image_height);

	for (int i = 0; i < data_size; i++) {
		new_data[i] = sqrt((pow(new_data_gx[i], 2) + pow(new_data_gy[i], 2)));
		if (new_data_gx[i] == 0)
			pixel_theta[i] = M_PI / 2;
		else
			pixel_theta[i] = atan(new_data_gy[i] / new_data_gx[i]);
	}

	delete[] new_data_gx;
	delete[] new_data_gy;
}

void Game::non_max_suppression(unsigned char* data, unsigned char* new_data, float* pixel_theta, int image_width, int image_height){

	const int data_size = 4 * image_height * image_width;
	int index_of_neighbor_in_1d_array;

	std::map <int, std::vector<int>> angle_step = {
		{6, {-1, 1, 1, -1}},
		{4, {0, -1, 0, 1}},
		{2, {-1, -1, 1, 1}},
		{0, {-1, 0, 1, 0}}, 
		{8, {-1, 0, 1, 0}}
	};

	for (int i = 0; i < data_size; i++) {

		// 4 is (R, G ,B, alpha)
		int data_row_as_3d_mat = i / (4 * image_width);
		int data_column_as_3d_mat = (i % (4 * image_width)) / 4;
		int data_color_as_3d_mat = i % 4;

		// convert theta into int represinting its range
		float curr_theta = pixel_theta[i];
		int index_in_angle_step = ((curr_theta + (M_PI / 2)) / (M_PI / 8) + 1);
		index_in_angle_step -= (index_in_angle_step % 2);
		std::vector<int> steps;
		try {
			steps = angle_step.at(index_in_angle_step);
		}
		catch (std::out_of_range) {
			steps = {0, 0};
			continue;
		}

		// Compare edge strength of current pixel with edge strength of pixel in positive and negative gradient directions
		index_of_neighbor_in_1d_array = check_neighbor(data_row_as_3d_mat, data_column_as_3d_mat, data_color_as_3d_mat, steps[0], steps[1], image_width, image_height);
		if (index_of_neighbor_in_1d_array != -1 && data[i] < data[index_of_neighbor_in_1d_array]) {
			new_data[i] = 0;
			continue;
		}
		index_of_neighbor_in_1d_array = check_neighbor(data_row_as_3d_mat, data_column_as_3d_mat, data_color_as_3d_mat, steps[2], steps[3], image_width, image_height);
		if (index_of_neighbor_in_1d_array != -1 && data[i] < data[index_of_neighbor_in_1d_array]) {
			new_data[i] = 0;
			continue;
		}
		// preserve value
		new_data[i] = data[i];

	}
}

void Game::hysteresis(unsigned char* data, unsigned char* new_data, int image_width, int image_height, double low_threshold, double high_threshold) {
	
	const int data_size = 4 * image_height * image_width;
	unsigned char* top_left_to_bottom_right = new unsigned char[data_size]; 
	unsigned char* bottom_right_to_top_left = new unsigned char[data_size];
	unsigned char* top_right_to_bottom_left = new unsigned char[data_size];
	unsigned char* bottom_left_to_top_right = new unsigned char[data_size];


	std::vector<std::vector<int>>neighbors{
	{-1, 1},  // right up
	{1, -1},  // left down
	{0, -1},  // left
	{0, 1},   // right
	{-1, -1}, // left up
	{1, 1},   // right down
	{-1, 0},  // up
	{1, 0},   // down

	};
	
	std::vector<std::vector<int>>&neighbors_ref = neighbors;

	for (int i = 0; i < data_size; i++) {
		// top_left_to_bottom_right
		thresholding_pixel(data, top_left_to_bottom_right, neighbors_ref, i, image_width, image_height, low_threshold, high_threshold);

		// bottom_right_to_top_left
		thresholding_pixel(data, bottom_right_to_top_left, neighbors_ref, (data_size - 1) - i, image_width, image_height, low_threshold, high_threshold);
	}	

	int row = 0;
	while (row < image_height) {
		for (int column = (image_width * 4) - 1; column >= 0; column--) {
			// index in 1d array
			int index = row * image_width * 4 + column;

			// top_right_to_bottom_left
			thresholding_pixel(data, top_right_to_bottom_left, neighbors_ref, index, image_width, image_height, low_threshold, high_threshold);

			// bottom_left_to_top_right
			thresholding_pixel(data, bottom_left_to_top_right, neighbors_ref, (data_size - 1) - index, image_width, image_height, low_threshold, high_threshold);
		}

		row += 1;
	}

	for (int i = 0; i < data_size; i++) {
		new_data[i] = top_left_to_bottom_right[i] + bottom_right_to_top_left[i] + top_right_to_bottom_left[i] + bottom_left_to_top_right[i];
	}

	delete[] top_left_to_bottom_right;
	delete[] bottom_right_to_top_left;
	delete[] top_right_to_bottom_left;
	delete[] bottom_left_to_top_right;

}

void Game::thresholding_pixel(unsigned char* data, unsigned char* new_data, std::vector<std::vector<int>>& neighbors, int index, int width, int height, double low_threshold, double high_threshold) {
	const int is_edge = 255;
	const int is_not_edge = 0;
	const int data_size = 4 * height * width;

	int data_row_as_3d_mat;
	int data_column_as_3d_mat;
	int data_color_as_3d_mat;
	int index_of_neighbor_in_1d_array;
	bool weak_to_strong = false;


	// strong pixel
	if (data[index] > high_threshold) {
		new_data[index] = is_edge;
		return;
	}

	// unrelevant pixel
	else if (data[index] < low_threshold) {
		// do nothing ??
		new_data[index] = is_not_edge;
		return;
	}

	// weak pixel
	// 4 is (R, G ,B, alpha)
	data_row_as_3d_mat = index / (4 * width);
	data_column_as_3d_mat = (index % (4 * width)) / 4;
	data_color_as_3d_mat = index % 4;
	weak_to_strong = false;

	for (std::vector<int> neigbor : neighbors) {
		index_of_neighbor_in_1d_array = check_neighbor(data_row_as_3d_mat, data_column_as_3d_mat, data_color_as_3d_mat, neigbor[0], neigbor[1], width, height);
		if (index_of_neighbor_in_1d_array != -1 && data[index_of_neighbor_in_1d_array]  == 255) {
			new_data[index] = is_edge;
			weak_to_strong = true;
			return;
		}
	}

	// not an edge
	if (!weak_to_strong)
		new_data[index] = is_not_edge;
}

void Game::convolution(unsigned char* data, unsigned char* new_data, std::vector<std::vector<float>>& kernel, int data_size, int image_width, int image_height)
{
	const int kernel_size = kernel.size();
	const int middle = (kernel_size - 1) / 2;

	// generate matrix (in size of kernel) to be multiplied by kernel
	std::vector<std::vector<float>> matrix(kernel_size, std::vector<float>(kernel_size, 0));

	for (int data_i = 0; data_i < data_size; data_i++) {

		// 4 is (R, G ,B, alpha)
		int data_row_as_3d_mat = data_i / (4 * image_width);
		int data_column_as_3d_mat = (data_i % (4 * image_width)) / 4;
		int data_color_as_3d_mat = data_i % 4;

		for (int matrix_i = 0; matrix_i < kernel_size; matrix_i++) {

			int row = data_row_as_3d_mat - middle + matrix_i;
			for (int matrix_j = 0; matrix_j < kernel_size; matrix_j++) {

				int column = data_column_as_3d_mat - middle + matrix_j;
				if (row >= 0 && column >= 0 && row < image_height && column < image_width) {
					if (data_size <= row * image_width * 4 + column * 4 + data_color_as_3d_mat) {
						continue;
					}
					matrix[matrix_i][matrix_j] = data[row * image_width * 4 + column * 4 + data_color_as_3d_mat];
				}
			}
		}

		// multiply matrix[x][y] with kernel[x][y] and sum up to "new pixel"
		float new_pixel = 0;
		for (int matrix_i = 0; matrix_i < kernel_size; matrix_i++) {
			for (int matrix_j = 0; matrix_j < kernel_size; matrix_j++) {
				new_pixel += matrix[matrix_i][matrix_j] * kernel[matrix_i][matrix_j];
			}
		}

		// update
		new_data[data_i] = (char)(abs((int)new_pixel));
	}
}
int Game::check_neighbor(int row, int column, int pixel, int up_down, int left_right, int width, int height) {
	int new_row = row + up_down;
	int new_column = column + left_right;
	int new_pixel = pixel;
	int neighbor_index = new_row * width * 4 + new_column * 4 + new_pixel;

	if ((0 <= new_row && new_row < height) && (0 <= new_column && new_column < width)) {
		return neighbor_index;
	}

	return -1;

}

void Game::halftone_pixel(unsigned char* data, unsigned char* new_data, int pixel_num, int width, std::vector<std::vector<unsigned char>>& halftone_patterns)
{
	int row_num_in_data = pixel_num / (4 * width / 2);
	int column_num_in_data = (pixel_num % (4 * width / 2)) / 4;

	int row_num_in_new_data = 2 * row_num_in_data;
	int column_num_in_new_data = 2 * column_num_in_data;
	
	for (int color = 0; color < 4; color++) {

		int halftone_pattern_index = ((float)data[pixel_num + color]/256) * 5; // this is the index of the halftone option for the ith color of this pixel (e.g. if the value is under 64 the option will be 0)

		std::vector<unsigned char> halftone_pattern = halftone_patterns[halftone_pattern_index];

		new_data[4 * width * row_num_in_new_data + 4 * column_num_in_new_data + color] = halftone_pattern[0];		
		new_data[4 * width * row_num_in_new_data + 4 * column_num_in_new_data + color + 4] = halftone_pattern[1];
		new_data[4 * width * (row_num_in_new_data + 1) + 4 * column_num_in_new_data + color] = halftone_pattern[2];
		new_data[4 * width * (row_num_in_new_data + 1) + 4 * column_num_in_new_data + color + 4] = halftone_pattern[3];

	}
}

void Game::halftone(unsigned char* data, unsigned char* new_data, int width, int height)
{

	std::vector<std::vector<unsigned char>> halftone_patterns
	{
		{0, 0, 0, 0},
		{0, 0, 255, 0},
		{0, 255, 255, 0},
		{0, 255, 255, 255},
		{255, 255, 255, 255},
	};
	
	for (int i = 0; i < width/2 * height/2 * 4; i+=4) {
		halftone_pixel(data, new_data, i, width, halftone_patterns);
	}
	
}

void Game::floyd_steinberg_pixel(std::vector<std::vector<float>>& new_data_float_values, int row_num, int column_num, int width, int height, std::vector<float>& colors)
{
	float original_color = new_data_float_values[row_num][column_num];
	float new_color = colors[(int)(original_color / 16)];
	float diff = original_color - new_color;

	new_data_float_values[row_num][column_num] = new_color;
	
	if (column_num + 4 < width * 4)
		new_data_float_values[row_num][column_num + 4] += diff * 7 / 16;
	
	if (row_num + 1 < height) {
		if (column_num - 4 >= 0)
			new_data_float_values[row_num + 1][column_num - 4] += diff * 3 / 16;

		new_data_float_values[row_num + 1][column_num] += diff * 5 / 16;

		if (column_num + 4 < width * 4)
			new_data_float_values[row_num + 1][column_num + 4] += diff * 1 / 16;
	}
	
}

void Game::floyd_steinberg(unsigned char* data, unsigned char* new_data, int width, int height)
{
	int data_size = width * height * 4;

	std::vector<std::vector<float>> new_data_float_values(height, std::vector<float>(width * 4)); //a matrix representing the float values of new_data
	std::vector<std::vector<float>>& new_data_float_values_ref = new_data_float_values; //reference of the vector to pass into floyd_steinberg_pixel

	std::vector<float> colors(16);
	for (int i = 0; i < 16; i++) {
		colors[i] = ((float)i) * 256 / 16; //the 16 color options
	}
	
	for (int i = 0; i < data_size; i++) {
		int row_num = i / (4 * width);
		int column_num = (i % (4 * width)) / 4;
		int color = i % 4;

		new_data_float_values[row_num][column_num + color] = (float)((int)data[i]); //give the pixel it's original color
		floyd_steinberg_pixel(new_data_float_values_ref, row_num, column_num + color, width, height, colors); //give the pixel it's new color and propagate the error
		new_data[i] = (unsigned char)((int)new_data_float_values[row_num][column_num + color]); //convert the pixel's new color to unsigned char and enter it in new_data

	}
}

void Game::print_matrix(unsigned char* data, int width, int height)
{
	std::ofstream matrix_file;
	matrix_file.open("matrix_file.txt");

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width * 4; j+=4) {
			matrix_file << (int)data[i * width * 4 + j] << " " << (int)data[i * width * 4 + j + 1] << " " << (int)data[i * width * 4 + j + 2] << " " << (int)data[i * width * 4 + j + 3] << "   ";
		}
		matrix_file << "\n\n";
	}

	matrix_file.close();
}

void Game::Update(const glm::mat4 &MVP,const glm::mat4 &Model,const int  shaderIndx)
{
	Shader *s = shaders[shaderIndx];
	int r = ((pickedShape+1) & 0x000000FF) >>  0;
	int g = ((pickedShape+1) & 0x0000FF00) >>  8;
	int b = ((pickedShape+1) & 0x00FF0000) >> 16;
	s->Bind();
	s->SetUniformMat4f("MVP", MVP);
	s->SetUniformMat4f("Normal",Model);
	s->SetUniform4f("lightDirection", 0.0f , 0.0f, -1.0f, 0.0f);
	if(shaderIndx == 0)
		s->SetUniform4f("lightColor",r/255.0f, g/255.0f, b/255.0f,1.0f);
	else 
		s->SetUniform4f("lightColor",0.7f,0.8f,0.1f,1.0f);
	s->Unbind();
}

void Game::WhenRotate()
{
}

void Game::WhenTranslate()
{
}

void Game::Motion()
{
	if(isActive)
	{
	}
}

Game::~Game(void)
{
}
