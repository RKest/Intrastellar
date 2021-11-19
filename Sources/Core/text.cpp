#include "Core/text.h"

void Text::Construct(const std::string &fontPath)
{
	if (FT_Init_FreeType(&ftl))
		std::cerr << "ERROR:FREETYPE: Failed to initialize freetype" << std::endl;
	if (FT_New_Face(ftl, fontPath.c_str(), 0, &face))
		std::cerr << "ERROR:FREETYPE: Failed to load fontface at: " << fontPath << std::endl; 
	FT_Set_Pixel_Sizes(face, 0, 48);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (uc c = 0; c < 128; ++c)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cerr << "ERROR:FREETYPE: Failed to load char: " << c << std::endl;
			continue;
		}
		
		ui texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		Character character = { texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), 
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), static_cast<ui>(face->glyph->advance.x) };
		characters.insert(std::pair<char, Character>(c, character));
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	shader.Bind();
	shader.SetUni("projection", projection);

	glGenVertexArrays(1, &vertexArrayObject);
	glGenBuffers(1, &vertexArrayBuffer);
	glBindVertexArray(vertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ft) * 24, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ft) * 4,  0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Text::Render(const std::string &text, ft x, const ft y, const ft scale, const glm::vec3 &colour)
{
	shader.Bind();
	shader.SetUni("texColour", colour);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vertexArrayObject);

	for (std::string::const_iterator c = text.begin(); c != text.end(); ++c)
	{
		const Character *character = &characters[*c];
		const ft xpos = x + decl_cast(xpos, character->bearing.x) * scale;
		const ft ypos = y - decl_cast(ypos, (character->size.y 	- character->bearing.y)) * scale;
		const ft width 	= 	decl_cast(width, character->size.x) * scale;
		const ft height = 	decl_cast(height, character->size.y) * scale;
		const ft vertices[6][4] = {
			{xpos, 		   ypos + height, 0.0f, 0.0f},
			{xpos, 		   ypos,          0.0f, 1.0f},
			{xpos + width, ypos,		  1.0f, 1.0f},
			{xpos, 		   ypos + height, 0.0f, 0.0f},
			{xpos + width, ypos,          1.0f, 1.0f},
			{xpos + width, ypos + height, 1.0f, 0.0f}
		};

		glBindTexture(GL_TEXTURE_2D, character->texId);
		glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += decl_cast(x, (character->advance >> 6)) * scale;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// Text::~Text(4dd)
// {
// 	FT_Done_Face(face);
// 	FT_Done_FreeType(ftl);
// }
