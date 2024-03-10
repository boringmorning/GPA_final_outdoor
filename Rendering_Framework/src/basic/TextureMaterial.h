#pragma once

#include <iostream>
#include <experimental\filesystem>
#include "IMaterial.h"



class TextureMaterial: public IMaterial
{
private:
	GLuint m_textureHandle;
	// 2D or 3D texture, default is 2D
	GLenum m_textureTarget = GL_TEXTURE_2D;
	// default is TEXTURE0
	GLenum m_textureUnit = GL_TEXTURE0;
	bool m_isEmpty = true;

public:
	TextureMaterial(const std::string &imageFile);	
	TextureMaterial(const void *data, const int comp, const int width, const int height, const GLint internalFormat, const GLenum imageFormat, const GLenum type);
	// for dynamic texture
	TextureMaterial(const GLuint handle, const GLenum textureUnit);
	// 2d texture array
	TextureMaterial(const void *data, const int comp, const int width, const int height, const int numLayer, const GLint internalFormal, const GLenum imageFormat, const GLenum type);


	bool isEmpty();
	virtual ~TextureMaterial();

	void bind() override;

	static unsigned char* loadImage(const std::string &image, int *width, int *height, int *comp);
	static unsigned char* loadPNG(const std::string &image, int *width, int *height, int *comp);
	static void loadImage(const std::string &image, unsigned char *dataBuffer);	
	static void removeImage(unsigned char* image);

	void setTextureUnit(const GLenum textureUnit);	
	bool empty() const;

	GLuint handle() const;

private:
	void createTexture(const void *data, const int width, const int height, const GLint internalFormat, const GLenum imageFormat, const GLenum type);

public:
	int m_width = -1;
	int m_height = -1;
};

