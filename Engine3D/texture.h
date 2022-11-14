#ifndef TEXTURE_H
#define TEXTURE_H
#include <vector>
#include <string>
#include <vector>

class Texture
{
public:
	Texture(const std::string& fileName);
	Texture(int width, int height,unsigned char *data);
	Texture(const std::string& fileName, std::string effect, unsigned int halftone_parameter); //constructor of a texture with a specific effect base on an image
	void Bind(int slot);
	inline int GetSlot(){return m_texture;}
	 ~Texture();
protected:
private:
	Texture(const Texture& texture) {}
	void operator=(const Texture& texture) {}
	unsigned int m_texture;
	void initialization(unsigned char* data, int width, int height);
	void edge_detection(unsigned char* data, int width, int height, int numComponents);
	void halftone_pixel(unsigned char* data, unsigned char* new_data, int pixel_num, int numComponents, int width, std::vector<std::vector<unsigned char>> halftone_options);
	void halftone(unsigned char* data, int width, int height, int numComponents);
	void floyd_steinberg(unsigned char* data, int width, int height, int numComponents);
	unsigned char* gaussian_blur(unsigned char* data, unsigned char* filtered_data, std::vector<std::vector<float>> gaussian_kernel, int data_size, int image_width, int image_height);

};

#endif
