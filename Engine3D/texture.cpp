//#define GLEW_STATIC
#include "texture.h"
#include "glad/include/glad/glad.h"
#include "stb_image.h"
#include <iostream>
#include <string>
#include <map>

Texture::Texture(const std::string& fileName)
{
	int width, height, numComponents;
    unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4);
	
    if(data == NULL)
		std::cerr << "Unable to load texture: " << fileName << std::endl;
        
    glGenTextures(1, &m_texture);
    Bind(m_texture);
        
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_LOD_BIAS,-0.4f);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
}

Texture::Texture(int width,int height,unsigned char *data)
{
    glGenTextures(1, &m_texture);
    Bind(m_texture);
        
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
}

Texture::Texture(const std::string& fileName, std::string effect, unsigned int halftone_parameter)
{

    typedef void (Texture::*pfunc)(const std::string&, unsigned int); //pfunc is the type of the effect functions

    std::map<std::string, pfunc> effects;
    effects["edge_detection"] = &Texture::edge_detection;
    effects["halftone"] = &Texture::halftone;
    effects["floyd_steinberg"] = &Texture::floyd_steinberg;

    try {
        pfunc func = effects.at(effect);
        (this->*func)(fileName, halftone_parameter);
    }
    catch (std::out_of_range) {
        std::cerr << "No such effect: " << effect << std::endl;
    };

}

Texture::~Texture()
{
	glDeleteTextures(1, &m_texture);
}

void Texture::edge_detection(const std::string& fileName, unsigned int)
{
    //niv

    int width, height, numComponents;
    unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4); //extract data from the image

    if (data == NULL)
        std::cerr << "Unable to load texture: " << fileName << std::endl;

    Texture(width, height, data);
}



void Texture::halftone(const std::string& fileName, unsigned int halftone_parameter)
{
    int width, height, numComponents;
    unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4); //extract data from the image

    if (data == NULL)
        std::cerr << "Unable to load texture: " << fileName << std::endl;

    unsigned char* new_data = new unsigned char[4 * sizeof(data)];

    Texture(width, height, data);
}

void Texture::floyd_steinberg(const std::string& fileName, unsigned int)
{
    int width, height, numComponents;
    unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4); //extract data from the image

    if (data == NULL)
        std::cerr << "Unable to load texture: " << fileName << std::endl;

    Texture(width, height, data);
}

void Texture::Bind(int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

