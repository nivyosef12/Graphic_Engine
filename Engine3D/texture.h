#ifndef TEXTURE_H
#define TEXTURE_H
#include <vector>
#include <string>

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
	void edge_detection(const std::string& fileName, unsigned int halftone_parameter);
	unsigned char* gaussian_blur(unsigned char* data, unsigned char* filtered_data, std::vector<std::vector<int>> gaussian_kernel, int data_size, int image_width, int image_height);
	void halftone(const std::string& fileName, unsigned int halftone_parameter);
	void floyd_steinberg(const std::string& fileName, unsigned int halftone_parameter);

};

#endif
