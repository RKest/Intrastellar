#pragma once
#include "_config.h"
#include "GL/glew.h"
#include "Ext/stb_image.h"

class Texture
{
public:
	Texture(const ui noTextures);
	void Instance(std::string const&);
	void Bind();
	~Texture();
private:
	GLuint _textures[MAX_NO_TEXTURES];
	size_t _noTextures{};
	size_t _nextTextureIndex{};
};
