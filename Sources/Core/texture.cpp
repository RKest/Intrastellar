#include "Core/texture.h"


Texture::Texture(const ui noTextures) : _noTextures(noTextures)
{
	glGenTextures(_noTextures, _textures);
}

Texture::~Texture()
{
    glDeleteTextures(_noTextures, _textures);
}

void Texture::Instance(std::string const& path)
{
	si width, height, noComponents;
	stbi_uc *imageData = stbi_load(path.c_str(), &width, &height, &noComponents, 4);
	
	if(imageData == NULL)
		throw std::runtime_error("No Image at: " + path);

	glBindTexture(GL_TEXTURE_2D, _textures[_nextTextureIndex++]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

	stbi_image_free(imageData);
}

void Texture::Bind()
{
	for(size_t i = 0; i < _noTextures; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, _textures[i]);
	}
}