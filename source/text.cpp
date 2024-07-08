#include "text.h"
#include "display.h"
#include "uiface.h"

#include <ft2build.h>
#include FT_FREETYPE_H

void draw_character(fontchar_t fontchar, int x, int y, unsigned char r, unsigned char g, unsigned char b);
void draw_text_line(const char* text, int x, int y, bool centered, unsigned char r, unsigned char g, unsigned char b);

Font* defaultFont = nullptr;
Font* buttonFont = nullptr;

static FT_Library freetype = nullptr;
static Font* currentFont = nullptr;

Font::Font(const char* filename, unsigned short size) {
	FT_Face face;
	short ofsY = size * 3 / 4;
	
	m_size = size;
	FT_New_Face(freetype, filename, 0, &face);
	FT_Set_Pixel_Sizes(face, 0, size);

	for (int i = 0; i < 128; i++) {
		FT_Load_Char(face, i, FT_LOAD_RENDER);
		unsigned short width = face->glyph->bitmap.width;
		unsigned short height = face->glyph->bitmap.rows;

		m_characters[i].ofsX = face->glyph->bitmap_left;
		m_characters[i].ofsY = face->glyph->bitmap_top - ofsY;
		m_characters[i].w = width;
		m_characters[i].h = height;
		m_characters[i].adv = face->glyph->advance.x >> 6;
		
		int area = width * height;
		unsigned char* image = new unsigned char[area * 4];
		for (int k = 0; k < area; k++) {
			int idx = k * 4;
			image[idx] = 255;
			image[idx + 1] = 255;
			image[idx + 2] = 255;
			image[idx + 3] = face->glyph->bitmap.buffer[k];
		}

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		m_characters[i].texture = texture;

		delete[] image;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
}

Font::~Font() {
	for (int i = 0; i < 128; i++)
		glDeleteTextures(1, &m_characters[i].texture);
}

unsigned short Font::getSize() {
	return m_size;
}

fontchar_t Font::getCharacter(char ascii) {
	return m_characters[ascii];
}

void text_initialize() {
	FT_Init_FreeType(&freetype);
	defaultFont = new Font("res/fonts/generic_condensed.ttf", 16);
	buttonFont = new Font("res/fonts/typewriter.ttf", 16);
}

void text_shutdown() {
	delete buttonFont;
	delete defaultFont;
	FT_Done_FreeType(freetype);
}

void set_text_font(Font* font) {
	currentFont = font;
}

int get_text_width(const char* text) {
	int i = 0;
	char ch = text[i];
	int width = 0;

	while (ch && ch != '\n') {
		width += currentFont->getCharacter(ch).adv;
		ch = text[++i];
	}

	return width;
}

void draw_text(const char* text, int x, int y, bool centered, unsigned char r, unsigned char g, unsigned char b) {
	int linebase = 0;
	int i = 0;
	char ch = text[i];
	int ofsY = 0;

	while (1) {
		if (!ch || ch == '\n') {
			draw_text_line(&text[linebase], x, y + ofsY, centered, r, g, b);
			if (ch == '\n')
				ofsY += currentFont->getSize();
			else
				break;
			linebase = i + 1;
		}
		ch = text[++i];
	}
}

void draw_character(fontchar_t fontchar, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
	int originX = x + fontchar.ofsX;
	int originY = y - fontchar.ofsY;

	glBindTexture(GL_TEXTURE_2D, fontchar.texture);
	glBegin(GL_TRIANGLE_FAN);
	glColor3ub(r, g, b);

	glTexCoord2i(0, 0);
	glVertex2f(XNDC(originX), YNDC(originY));
	glTexCoord2i(0, 1);
	glVertex2f(XNDC(originX), YNDC(originY + fontchar.h));
	glTexCoord2i(1, 1);
	glVertex2f(XNDC(originX + fontchar.w), YNDC(originY + fontchar.h));
	glTexCoord2i(1, 0);
	glVertex2f(XNDC(originX + fontchar.w), YNDC(originY));

	glEnd();
}

void draw_text_line(const char* text, int x, int y, bool centered, unsigned char r, unsigned char g, unsigned char b) {
	int i = 0;
	char ch = text[i];
	int width = centered ? get_text_width(text) : 0;
	int ofsX = width / -2;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (ch && ch != '\n') {
		fontchar_t character = currentFont->getCharacter(ch);
		draw_character(character, x + ofsX, y, r, g, b);
		ofsX += character.adv;
		ch = text[++i];
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}