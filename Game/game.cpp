#define _USE_MATH_DEFINES

#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <cmath>

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

	std::string fileName = "../res/textures/lena256.jpg";
	int width, height, numComponents;
	unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4); //extract data from the image
	
	unsigned char* new_data = edge_detection(width, height, data);
	AddTexture(width, height, new_data);
	// AddTexture("../res/textures/lena256.jpg",false);
	//AddTexture();

	stbi_image_free(data);
	// AddTexture("../res/textures/lena256.jpg","edge_detection", 4);
	// AddTexture("../res/textures/lena256.jpg",false);
	// AddTexture("../res/textures/lena256.jpg", "edge_detection", 0);

	AddShape(Plane,-1,TRIANGLES);
	
	pickedShape = 0;
	
	SetShapeTex(0,0);
	MoveCamera(0,zTranslate,1.5);
	pickedShape = -1;
	
	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

unsigned char* Game::edge_detection(int width, int height, unsigned char* data)
{
	unsigned char* smoothed_data = new unsigned char[4 * width * height]; //TODO: memory leaks!!
	unsigned char* derived_data = new unsigned char[4 * width * height]; //TODO: memory leaks!!
	float* pixel_theta = new float[4 * width * height]; //TODO: memory leaks!!
	unsigned char* non_max_suppression_data = new unsigned char[4 * width * height]; //TODO: memory leaks!!


	smoothing(data, smoothed_data, width, height);
	derivative(smoothed_data, derived_data, pixel_theta, width, height);
	// non_max_suppression(derived_data, non_max_suppression_data, pixel_theta, width, height);
	

	// TODO free data
	return derived_data;
}

void Game::smoothing(unsigned char* data, unsigned char* new_data, int image_width, int image_height) {

	// generate Gaussian filter kernel of size (2k +1)X(2k + 1)
	const int data_size = 4 * image_height * image_width;
	const int k = 2;
	const int kernel_size = (2 * k) + 1;
	const double sigma = 0.97;
	std::vector<std::vector<float>> gaussian(kernel_size, std::vector<float>(kernel_size, 0));

	for (int i = 0; i < kernel_size; i++) {
		for (int j = 0; j < kernel_size; j++) {

			float normal = 1 / (2.0 * M_PI * pow(sigma, 4));
			gaussian[i][j] = normal * exp(-((pow(i - k, 2) + pow(j - k, 2)) / (2.0 * pow(sigma, 2))));

		}
	}

	convolution(data, new_data, gaussian, data_size, image_width, image_height);  // smoothing with gaussian filter
}

void Game::derivative(unsigned char* data, unsigned char* new_data, float* pixel_theta, int image_width, int image_height) {

	const int data_size = 4 * image_height * image_width;
	unsigned char* new_data_gx = new unsigned char[data_size]; //TODO: memory leaks!!
	unsigned char* new_data_gy = new unsigned char[data_size]; //TODO: memory leaks!!

	/*std::vector<std::vector<float>> Gx{
	{-1, 0, 1},
	{-2, 0, 2},
	{-1, 0, 1}
	};
	std::vector<std::vector<float>> Gy{
	{-1, -2, -1},
	{0, 0, 0},
	{1, 2, 1}
	};*/

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

	convolution(data, new_data_gx, Gx, data_size, image_width, image_height);
	convolution(data, new_data_gy, Gy, data_size, image_width, image_height);

	for (int i = 0; i < data_size; i++) {
		new_data[i] = sqrt((pow(new_data_gx[i], 2) + pow(new_data_gy[i], 2)));
		if (new_data_gx[i] == 0)
			pixel_theta[i] = M_PI / 2;
		else
			pixel_theta[i] = atan(new_data_gy[i] / new_data_gx[i]);
	}
	// TODO free gx, gy
}


void Game::convolution(unsigned char* data, unsigned char* new_data, std::vector<std::vector<float>> gaussian_kernel, int data_size, int image_width, int image_height)
{
	const int kernel_size = gaussian_kernel.size();
	const int middle = (kernel_size - 1) / 2;

	for (int data_i = 0; data_i < data_size; data_i++) {

		// 4 is (R, G ,B, alpha)
		int data_row_as_3d_mat = data_i / (4 * image_width);
		int data_column_as_3d_mat = (data_i % (4 * image_width)) / 4;
		int data_color_as_3d_mat = data_i % 4;

		// generate matrix (in size of gaussian_kernel) to be multiplied by gaussian_kernel
		std::vector<std::vector<float>> matrix(kernel_size, std::vector<float>(kernel_size, 0));
		for (int matrix_i = 0; matrix_i < kernel_size; matrix_i++) {

			int row = data_row_as_3d_mat - middle + matrix_i;
			for (int matrix_j = 0; matrix_j < kernel_size; matrix_j++) {

				int column = data_column_as_3d_mat - middle + matrix_j;
				if (row >= 0 && column >= 0)
					matrix[matrix_i][matrix_j] = data[row * image_width * 4 + column * 4 + data_color_as_3d_mat];
			}
		}

		// multiply matrix[x][y] with gaussian_kernel[x][y] and sum up to "new pixel"
		float new_pixel = 0;
		for (int matrix_i = 0; matrix_i < kernel_size; matrix_i++) {
			for (int matrix_j = 0; matrix_j < kernel_size; matrix_j++) {
				new_pixel += matrix[matrix_i][matrix_j] * gaussian_kernel[matrix_i][matrix_j];
			}
		}

		// update
		new_data[data_i] = (char)(abs((int)new_pixel));
	}
}

void Game::non_max_suppression(unsigned char* data, unsigned char* new_data, float* pixel_theta, int image_width, int image_height){
	const int data_size = 4 * image_height * image_width;
	int neighbor_x;
	int neighbor_y;
	int neighbor_pixel;
	int index_of_neighbor_in_1d_array;
	for (int i = 0; i < data_size; i++) {

		// 4 is (R, G ,B, alpha)
		int data_row_as_3d_mat = i / (4 * image_width);
		int data_column_as_3d_mat = (i % (4 * image_width)) / 4;
		int data_color_as_3d_mat = i % 4;

		float curr_theta = pixel_theta[i];

		if (curr_theta <= 3 * M_PI / 8 && curr_theta >= M_PI / 8) {
			// right up
			neighbor_x = data_row_as_3d_mat - 1;
			neighbor_y = data_column_as_3d_mat + 1;
			neighbor_pixel = data_color_as_3d_mat;
			index_of_neighbor_in_1d_array = neighbor_x * image_width * 4 + neighbor_y * 4 + neighbor_pixel;
			if (neighbor_x >= 0 && neighbor_y <= image_height && data[i] < data[index_of_neighbor_in_1d_array]) {
				new_data[i] = 0;
				continue;
			}

			// left down
			neighbor_x = data_row_as_3d_mat + 1;
			neighbor_y = data_column_as_3d_mat - 1;
			neighbor_pixel = data_color_as_3d_mat;
			index_of_neighbor_in_1d_array = neighbor_x * image_width * 4 + neighbor_y * 4 + neighbor_pixel;
			if (neighbor_x < image_height && neighbor_y >= 0 && data[i] < data[index_of_neighbor_in_1d_array]) {
				new_data[i] = 0;
				continue;
			}

		}
		else if (curr_theta <= M_PI / 8 && curr_theta >=  -M_PI / 8) {
			// left neighbor
			neighbor_x = data_row_as_3d_mat;
			neighbor_y = data_column_as_3d_mat - 1;
			neighbor_pixel = data_color_as_3d_mat;
			index_of_neighbor_in_1d_array = neighbor_x * image_width * 4 + neighbor_y * 4 + neighbor_pixel;
			if (neighbor_y >= 0 && data[i] < data[index_of_neighbor_in_1d_array]) {
				new_data[i] = 0;
				continue;
			}

			// right neighbor
			neighbor_x = data_row_as_3d_mat;
			neighbor_y = data_column_as_3d_mat + 1;
			neighbor_pixel = data_color_as_3d_mat;
			index_of_neighbor_in_1d_array = neighbor_x * image_width * 4 + neighbor_y * 4 + neighbor_pixel;
			if (neighbor_y < image_width && data[i] < data[index_of_neighbor_in_1d_array]) {
				new_data[i] = 0;
				continue;
			}

		}
		else if (curr_theta <= -M_PI / 8 && curr_theta >= -3 * M_PI / 8) {
			// left up
			neighbor_x = data_row_as_3d_mat - 1;
			neighbor_y = data_column_as_3d_mat - 1;
			neighbor_pixel = data_color_as_3d_mat;
			index_of_neighbor_in_1d_array = neighbor_x * image_width * 4 + neighbor_y * 4 + neighbor_pixel;
			if (neighbor_y >= 0 && neighbor_x >= 0 && data[i] < data[index_of_neighbor_in_1d_array]) {
				new_data[i] = 0;
				continue;
			}

			// right down
			neighbor_x = data_row_as_3d_mat + 1;
			neighbor_y = data_column_as_3d_mat + 1;
			neighbor_pixel = data_color_as_3d_mat;
			index_of_neighbor_in_1d_array = neighbor_x * image_width * 4 + neighbor_y * 4 + neighbor_pixel;
			if (neighbor_y < image_width && neighbor_x < image_height && data[i] < data[index_of_neighbor_in_1d_array]) {
				new_data[i] = 0;
				continue;
			}

		}
		else {

			// up neighbor
			neighbor_x = data_row_as_3d_mat - 1;
			neighbor_y = data_column_as_3d_mat;
			neighbor_pixel = data_color_as_3d_mat;
			index_of_neighbor_in_1d_array = neighbor_x * image_width * 4 + neighbor_y * 4 + neighbor_pixel;
			if (neighbor_x >= 0 && data[i] < data[index_of_neighbor_in_1d_array]) {
				new_data[i] = 0;
				continue;
			}

			// down neighbor
			neighbor_x = data_row_as_3d_mat + 1;
			neighbor_y = data_column_as_3d_mat;
			neighbor_pixel = data_color_as_3d_mat;
			index_of_neighbor_in_1d_array = neighbor_x * image_width * 4 + neighbor_y * 4 + neighbor_pixel;
			if (neighbor_x < image_height && data[i] < data[index_of_neighbor_in_1d_array]) {
				new_data[i] = 0;
				continue;
			}
		}

		// preserve value
		new_data[i] = data[i];

	}
}


void Game::halftone_pixel(unsigned char* data, unsigned char* new_data, int pixel_num, int width, std::vector<std::vector<unsigned char>> halftone_options)
{
	for (int i = 0; i < 4; i++) {
		int data_inti = (int)data[pixel_num + i];
		int option = (((int)data[pixel_num + i]) / 4) / (256 / 4); // this is the index of the halftone option for the ith color of this pixel (e.g. if the value is under 64 the option will be 0)
		std::vector<unsigned char> halftone_option = halftone_options[option];
		new_data[2 * pixel_num] = halftone_option[0];
		new_data[2 * (pixel_num + 4)] = halftone_option[1];
		new_data[2 * pixel_num + 4 * width] = halftone_option[2];
		new_data[2 * (pixel_num + 4) + 4 * width] = halftone_option[3];
	}
}

void Game::halftone(int* width, int* height, unsigned char* data)
{
	unsigned char* new_data = new unsigned char[*width * *height * 4]; //TODO: memory leaks!!

	std::vector<std::vector<unsigned char>> halftone_options
	{
		{(unsigned char)0, (unsigned char)0, (unsigned char)0, (unsigned char)0},
		{(unsigned char)0, (unsigned char)0, (unsigned char)255, (unsigned char)0},
		{(unsigned char)0, (unsigned char)255, (unsigned char)255, (unsigned char)0},
		{(unsigned char)0, (unsigned char)255, (unsigned char)255, (unsigned char)255},
		{(unsigned char)255, (unsigned char)255, (unsigned char)255, (unsigned char)255},
	};

	for (int i = 0; i < *width * *height * 4; i++) {
		halftone_pixel(data, new_data, i, *width, halftone_options);
	}

	std::cout << "got here" << std::endl;

	/*for (int i = 0; i < vec.size(); i++) {
		std::cout << vec[i] << std::endl;
	}*/

}

void Game::floyd_steinberg(int width, int height, unsigned char* data)
{

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
