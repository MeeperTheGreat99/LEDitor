#pragma once

/* Font Character */
typedef struct fontchar_s {
	short ofsX, ofsY;
	unsigned short w, h;
	unsigned short adv;
	unsigned int texture;
} fontchar_t;

class Font {
private:
	unsigned short m_size;
	fontchar_t m_characters[128];

public:
	Font(const char* filename, unsigned short size);
	~Font();

	unsigned short getSize();
	fontchar_t getCharacter(char ascii);
};

void text_initialize();
void text_shutdown();
void set_text_font(Font* font);
int get_text_width(const char* text);
int get_text_width_max(const char* text);
int get_text_height(const char* text);
void draw_text(const char* text, int x, int y, bool centered = false, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255);

extern Font* defaultFont;
extern Font* defaultSmFont;
extern Font* buttonFont;