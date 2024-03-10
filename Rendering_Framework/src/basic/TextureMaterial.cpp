#include "TextureMaterial.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

TextureMaterial::TextureMaterial(const GLuint handle, const GLenum textureUnit) : IMaterial(GL_TRIANGLES, IMaterial::TEXTURE) {
	this->m_textureHandle = handle;
	this->m_textureUnit = textureUnit;
}
TextureMaterial::TextureMaterial(const std::string &imageFile) : IMaterial(GL_TRIANGLES, IMaterial::TEXTURE)
{
	// Read Texture
	int width, height, comp;
	unsigned char *textureData = TextureMaterial::loadImage(imageFile.c_str(), &width, &height, &comp);
	if (!textureData){
		std::cout << "The Texture \"" << imageFile << "\" is not found\n";
		m_isEmpty = true;
		return;
	}

	// use fast 4-byte alignment (default anyway) if possible
	if ((width * comp) % 4 != 0){
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}
	if (comp == 3){
		this->createTexture(textureData, width, height, GL_RGBA, GL_RGB, GL_UNSIGNED_BYTE);
	}
	else if (comp == 4){
		this->createTexture(textureData, width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	}	
	else{
		std::cout << "unknown pixel storage\n";
		stbi_image_free(textureData);
		m_isEmpty = true;
		return;
	}

	// release image 
	stbi_image_free(textureData);

	// preserve the data in main memory
	this->m_width = width;
	this->m_height = height;

	m_isEmpty = false;
}
void TextureMaterial::createTexture(const void *data, const int width, const int height, const GLint internalFormat, const GLenum imageFormat, const GLenum type){
	// Create Texture
	glGenTextures(1, &m_textureHandle);
	glBindTexture(GL_TEXTURE_2D, m_textureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, imageFormat, type, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//float aniso = 1.0;
	//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);

	glBindTexture(GL_TEXTURE_2D, 0);
}
// 2D texture array
TextureMaterial::TextureMaterial(const void *data, const int comp, const int width, const int height, const int numLayer, const GLint internalFormat, const GLenum imageFormat, const GLenum type) : IMaterial(GL_TRIANGLES, IMaterial::TEXTURE){
	glGenTextures(1, &(this->m_textureHandle));
	glBindTexture(GL_TEXTURE_2D_ARRAY, this->m_textureHandle);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat, width, height, numLayer, 0, imageFormat, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, numLayer);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	this->m_textureTarget = GL_TEXTURE_2D_ARRAY;

	// preserve the data in main memory
	this->m_width = width;
	this->m_height = height;

	m_isEmpty = false;
}

bool TextureMaterial::empty() const{
	return this->m_isEmpty;
}

TextureMaterial::TextureMaterial(const void *data, const int comp, const int width, const int height, const GLint internalFormat, const GLenum imageFormat, const GLenum type) : IMaterial(GL_TRIANGLES, IMaterial::TEXTURE){
	// use fast 4-byte alignment (default anyway) if possible
	if ((width * comp) % 4 != 0){
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}
	this->createTexture(data, width, height, internalFormat, imageFormat, type);
	

	// preserve the data in main memory
	this->m_width = width;
	this->m_height = height;

	m_isEmpty = false;
}

TextureMaterial::~TextureMaterial()
{
}

bool TextureMaterial::isEmpty(){
	return m_isEmpty;
}

void TextureMaterial::bind(){
	glActiveTexture(m_textureUnit);
	glBindTexture(m_textureTarget, this->m_textureHandle);
}
void TextureMaterial::setTextureUnit(const GLenum textureUnit) {
	this->m_textureUnit = textureUnit;
}
GLuint TextureMaterial::handle() const{
	return this->m_textureHandle;
}

////////////////////////////////////////////////////////////////////////////
unsigned char* TextureMaterial::loadImage(const std::string &image, int *width, int *height, int *comp){
	std::experimental::filesystem::path fi(image);
	std::string filename = fi.stem().generic_string();
	std::string direction = fi.parent_path().generic_string();
	std::string suffix = fi.extension().generic_string();

	if (suffix == ".png") {
		return TextureMaterial::loadPNG(image, width, height, comp); 
	}

	// Read Texture
	stbi_uc *textureData = stbi_load(image.c_str(), width, height, comp, 3);
	if (!textureData){
		std::cout << "The Texture \"" << image << "\" is not found\n";
		return nullptr;
	}
	return textureData;
}

unsigned char* TextureMaterial::loadPNG(const std::string &image, int *width, int *height, int *comp)
{
	int w, h, n;
	unsigned char *iData = nullptr;
	stbi_uc *data = stbi_load(image.c_str(), &w, &h, &n, 4);
	if (data != NULL)
	{
		iData = new unsigned char[w * h * 4 ];
		memcpy(iData, data, w * h * 4);
		// vertical-mirror image data
		for (size_t i = 0; i < w; i++)
		{
			for (size_t j = 0; j < h / 2; j++)
			{
				for (size_t k = 0; k < 4; k++) {
					std::swap(iData[(j * w + i) * 4 + k], iData[((h - j - 1) * w + i) * 4 + k]);
				}
			}
		}
		stbi_image_free(data);
	}
	*width = w;
	*height = h;
	*comp = n;
	return iData;
}


void TextureMaterial::loadImage(const std::string &image, unsigned char *dataBuffer){
	int width, height, comp;
	stbi_uc *textureData = stbi_load(image.c_str(), &width, &height, &comp, 3);
	if (!textureData){
		std::cout << "The Texture \"" << image << "\" is not found\n";
		return ;
	}
	for (int i = 0; i < width * height * comp; i++){
		dataBuffer[i] = textureData[i];
	}
	delete[] textureData;
}
void TextureMaterial::removeImage(unsigned char* image){
	stbi_image_free(image);
}