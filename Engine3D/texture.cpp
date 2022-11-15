//#define GLEW_STATIC
#include "texture.h"
#include "glad/include/glad/glad.h"
#include "stb_image.h"
#include <iostream>
#include <string>
#include <map>
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

    std::cout << "inside edge_detection function" << std::endl;


}

void Texture::halftone_pixel(unsigned char* data, unsigned char* new_data, int pixel_num, int numComponents,int width, std::vector<std::vector<unsigned char>> halftone_options)
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

void Texture::halftone(unsigned char* data, int width, int height, int numComponents)
{
    

    unsigned char* new_data = new unsigned char[width * height * 4]; //TODO: memory leaks!!
    
    std::vector<std::vector<unsigned char>> halftone_options
    {
        {(unsigned char)0, (unsigned char)0, (unsigned char)0, (unsigned char)0},
        {(unsigned char)0, (unsigned char)0, (unsigned char)255, (unsigned char)0},
        {(unsigned char)0, (unsigned char)255, (unsigned char)255, (unsigned char)0},
        {(unsigned char)0, (unsigned char)255, (unsigned char)255, (unsigned char)255},
        {(unsigned char)255, (unsigned char)255, (unsigned char)255, (unsigned char)255},
    };
        
    for (int i = 0; i < width * height * 4; i++) {
        halftone_pixel(data, new_data, i, numComponents, width, halftone_options);
    }

    std::cout << "got here" << std::endl;

    //std::vector<int> vec(new_data, new_data + width * height * 4);

    /*for (int i = 0; i < vec.size(); i++) {
        std::cout << vec[i] << std::endl;
    }*/

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

