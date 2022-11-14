#ifndef TEXTURE_H
#define TEXTURE_H

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
	void halftone(const std::string& fileName, unsigned int halftone_parameter);
	void floyd_steinberg(const std::string& fileName, unsigned int halftone_parameter);

};

#endif
