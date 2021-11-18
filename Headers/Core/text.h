#pragma once
#include "_config.h"
#include "shader.h"

#include "GL/glew.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include <iostream>
#include <string>
#include <map>

struct Character
{
	ui texId;
	glm::ivec2 size;
	glm::ivec2 bearing;
	ui advance;
};

class Text
{
public:
	static void Construct(const std::string &fontPath);
	static void Render(const std::string &text, ft x, const ft y, const ft scale, const glm::vec3 &colour);

protected:
private:
	inline static FT_Library ftl;
	inline static FT_Face face;
	inline static GLuint vertexArrayObject;
	inline static GLuint vertexArrayBuffer;

	inline static Shader shader{ "Shaders/Text" };
	inline static std::map<char, Character> characters;
	inline static const glm::mat4 projection = glm::ortho(0.0f, static_cast<ft>(SCREEN_WIDTH), 0.0f, static_cast<ft>(SCREEN_HEIGHT));
	
};