#pragma once
#include "bitmap.h"
#include <vector>
#include <functional>

#define MOUSE_LMB       (1 << 0)
#define MOUSE_RMB       (1 << 1)
#define MOUSE_MMB       (1 << 2)
#define MOUSE_XMB1      (1 << 3)
#define MOUSE_XMB2      (1 << 4)

/* Rectangle */
typedef struct rect_s {
	int x, y;
	int w, h;
	unsigned char r, g, b;
	unsigned int texture;
} rect_t;
rect_t* create_rect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
void destroy_rect(rect_t* rect);
void draw_rect(rect_t* rect);

class UIWidget {
private:
	rect_t* m_bounds;

protected:
	bool m_hovering;
	bool m_pressing;
	bool m_hoveringLast;
	bool m_pressingLast;
	bool m_hovered;
	bool m_unhovered;
	bool m_pressed;
	bool m_released;

public:
	UIWidget(int x, int y, int width, int height);
	~UIWidget();

	virtual void update();
	virtual void draw() {}
};

class UIScreen {
private:
	std::vector<UIWidget*> m_uiwidgets;

public:
	UIScreen();
	~UIScreen();

	void update();
	void draw();
	void addUIWidget(UIWidget* uiwidget);
	void removeUIWidget(UIWidget* uiwidget);
};

class UIRect : public UIWidget {
protected:
	rect_t* m_rect;

public:
	UIRect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
	~UIRect();

	void setColor(unsigned char r, unsigned char g, unsigned char b);

	virtual void draw() override;
};

class UILabel : public UIRect {
private:
	char m_text[64];

public:
	UILabel(const char* text, int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
	~UILabel();

	void setText(const char* text);

	virtual void draw() override;
};

class UIButton : public UIRect {
private:
	const char* m_text;
	rect_t* m_overlay;
	std::function<void(UIButton*)> m_clickFunc;

public:
	UIButton(const char* text, int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
	~UIButton();

	void setClickFunc(std::function<void(UIButton*)> clickFunc);

	virtual void update() override;
	virtual void draw() override;
};

class UISlider : public UIRect {
private:
	unsigned char m_minR, m_minG, m_minB;
	unsigned char m_maxR, m_maxG, m_maxB;
	unsigned char m_value;
	rect_t* m_tick;

public:
	UISlider(int x, int y, int width, int height);
	~UISlider();

	void setMaxColor(unsigned char r, unsigned char g, unsigned char b);
	unsigned char getValue();

	virtual void update() override;
	virtual void draw() override;
};

class UIEditBitmap : public UIRect {
private:
	bitmap_t* m_bitmap;
	unsigned int m_texture;
	int m_selectedOp;
	unsigned char m_selectedR;
	unsigned char m_selectedG;
	unsigned char m_selectedB;
	int m_mouseXStart;
	int m_mouseYStart;

public:
	UIEditBitmap(int x, int y, int width, int height, int imageWidth, int imageHeight);
	~UIEditBitmap();

	void clear();
	void reload(int width, int height, unsigned char* data);
	void setDrawColor(unsigned char r, unsigned char g, unsigned char b);
	void setDrawOperation(int op);
	int getImageWidth();
	int getImageHeight();
	unsigned char* getImageData();

	virtual void update() override;
	virtual void draw() override;

private:
	void regenTexture(bool first = false);
	void updateTexture();
};

void uiface_initialize();
void uiface_shutdown();
void uiface_update();
void uiface_draw();
void uiface_resize(int w, int h);
void uiface_set_screen(UIScreen* screen);
void uiface_mouse_position(int x, int y);
void uiface_mouse_buttons_down(int buttons);
void uiface_mouse_buttons_up(int buttons);

float uiface_px_size_x();
float uiface_px_size_y();
#define XNDC(x) (float(x) * uiface_px_size_x() - 1.0f)
#define YNDC(x) (float(x) * -uiface_px_size_y() + 1.0f)

extern std::vector<UIWidget*> gUIWidgets;