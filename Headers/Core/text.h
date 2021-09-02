#ifndef TEXT_H
#define TEXT_H
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
	Text(const std::string &fontPath, const ui screenWidth, const ui screenHeihgt);
	void Render(const std::string &text, ft x, const ft y, const ft scale, const glm::vec3 &colour);
	~Text();

protected:
private:
	FT_Library ftl;
	FT_Face face;
	GLuint vertexArrayObject;
	GLuint vertexArrayBuffer;

	Shader shader;
	std::map<char, Character> characters;
	glm::mat4 projection;
	
};
#endif