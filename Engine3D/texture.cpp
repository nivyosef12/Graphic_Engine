#define _USE_MATH_DEFINES
//#define GLEW_STATIC

#include "texture.h"
#include "glad/include/glad/glad.h"
#include "stb_image.h"
#include <iostream>
#include <string>
#include <map>
#include <cmath>
#include <vector>

Texture::Texture(const std::string& fileName)
{
    std::cout << "entered the fileName constructor" << std::endl;
	int width, height, numComponents;
    unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4);

    if(data == NULL)
		std::cerr << "Unable to load texture: " << fileName << std::endl;
        
    /*for (int i = 0; i < width*height*numComponents; i++) {
        std::cout << (int)data[i] << std::endl;
    }*/

    initialization(data, width, height);

   /* glGenTextures(1, &m_texture);
    Bind(m_texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/
    glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_LOD_BIAS,-0.4f);
    /*glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);*/
    stbi_image_free(data);
}

Texture::Texture(int width,int height,unsigned char *data)
{
    std::cout << "entered right constructor" << std::endl;
    
    /*glGenTextures(1, &m_texture);
    Bind(m_texture);*/
        
   /* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);*/

    initialization(data, width, height);
	
}

Texture::Texture(const std::string& fileName, std::string effect, unsigned int halftone_parameter)
{

    typedef void (Texture::*pfunc)(unsigned char*, int, int, int); //pfunc is the type of the effect functions

    std::map<std::string, pfunc> effects;
    effects["edge_detection"] = &Texture::edge_detection;
    effects["halftone"] = &Texture::halftone;
    effects["floyd_steinberg"] = &Texture::floyd_steinberg;

    int width, height, numComponents;
    unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4); //extract data from the image

    if (data == NULL)
        std::cerr << "Unable to load texture: " << fileName << std::endl;

    try {
        pfunc func = effects.at(effect);
        (this->*func)(data, width, height, numComponents);
    }
    catch (std::out_of_range) {
        std::cerr << "No such effect: " << effect << std::endl;
        initialization(data, width, height);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.4f);
    };
    
    stbi_image_free(data);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_texture);
}

void Texture::initialization(unsigned char* data, int width, int height)
{
    glGenTextures(1, &m_texture);
    Bind(m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void Texture::edge_detection(unsigned char* data, int width, int height, int numComponents)
{
    //niv
    // generate Gaussian filter kernel of size (2k +1)X(2k + 1)
    const int k = 2;
    const int kernel_size = (2 * k) + 1;
    const double sigma = 1.0;
    std::vector<std::vector<int>> gaussian(kernel_size, std::vector<int>(kernel_size, 0)); // TODO lond double??

    for (int i = 0; i < kernel_size; i++) {
        for (int j = 0; j < kernel_size; j++) {
            // TODO calculate determinet
            gaussian[i][j] = ((1 / 2 * M_PI * pow(sigma, 2)) *
                exp((-1 * (pow((i + 1) - (k + 1), 2) + pow((j + 1) - (k + 1), 2))) / 2 * pow(sigma, 2)) / 100);
        }
    }
    
    // TODO call blur

    Texture(width, height, data);
    // TODO free data
    
    std::cout << "inside edge_detection function" << std::endl;
}

// TODO change to void
unsigned char* gaussian_blur(unsigned char* data, unsigned char* filtered_data, std::vector<std::vector<int>> gaussian_kernel, int data_size, int image_width, int image_height)
{
    const int kernel_size = gaussian_kernel.size();
    const int middle = (kernel_size + 1) / 2;

    for (int data_i = 0; data_i < data_size; data_i++) {

        // 4 is (R, G ,B, alpha)
        int data_row_as_3d_mat = data_i / (4 * image_width);
        int data_column_as_3d_mat = (data_i % (4 * image_width)) / 4;
        int data_color_as_3d_mat = data_i % 4;

        // generate matrix (in size of gaussian_kernel) to be multiplied by gaussian_kernel
        std::vector<std::vector<int>> matrix(kernel_size, std::vector<int>(kernel_size, 0));
        matrix[middle][middle] = data[data_i];
        for (int matrix_i = 0; matrix_i < kernel_size; matrix_i++) {
            int row = data_row_as_3d_mat - middle + matrix_i;
            for (int matrix_j = 0; matrix_j < kernel_size; matrix_j++) {
                int column = data_column_as_3d_mat - middle + matrix_j;
                if (row >= 0 && column >= 0)
                    matrix[matrix_i][matrix_j] = data[row * image_width + column * 4 + data_color_as_3d_mat];
            }
        }

        // multiply matrix[x][y] with gaussian_kernel[x][y] and sum up to "new pixel"
        int new_pixel = 0;
        for (int matrix_i = 0; matrix_i < kernel_size; matrix_i++) {
            for (int matrix_j = 0; matrix_j < kernel_size; matrix_j++) {
                new_pixel += matrix[matrix_i][matrix_j] * gaussian_kernel[matrix_i][matrix_j];
            }
        }

        // filtered_data[i] = new pixel
        filtered_data[data_i] = (char)new_pixel;
    }

    return filtered_data;
}

void Texture::halftone_pixel(unsigned char* data, unsigned char* new_data, int pixel_num, int numComponents,int width, std::vector<std::vector<unsigned char>> halftone_options)
{
    for (int i = 0; i < numComponents; i++) {
        int option = (((int)data[pixel_num + i]) / 4) / (256 / 4); // this is the index of the halftone option for the ith color of this pixel (e.g. if the value is under 64 the option will be 0)
        std::vector<unsigned char> halftone_option = halftone_options[option];
        new_data[2 * pixel_num] = halftone_option[0];
        new_data[2 * pixel_num + numComponents] = halftone_option[1];
        new_data[2 *pixel_num + 4 * width] = halftone_option[2];
        new_data[2 * pixel_num + 4 * width + numComponents] = halftone_option[3];
    }
}

void Texture::halftone(unsigned char* data, int width, int height, int numComponents)
{
    

    unsigned char* new_data = new unsigned char[4 * sizeof(data)]; //TODO: memory leaks!!
    
    std::vector<std::vector<unsigned char>> halftone_options
    {
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 1},
        {1, 1, 1, 1},
    };
    
    for (int i = 0; i < sizeof(data); i++) {
        halftone_pixel(data, new_data, i, numComponents, width, halftone_options);
    }

    initialization(new_data, width*4, height*4);

}

void Texture::floyd_steinberg(unsigned char* data, int width, int height, int numComponents)

{
    initialization(data, width, height);

}

void Texture::Bind(int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

