#pragma once
#include "bitmap.h"
#include "text.h"
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
	unsigned char r, g, b, a;
	unsigned int texture;
} rect_t;
rect_t* create_rect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
void destroy_rect(rect_t* rect);
void draw_rect(rect_t* rect);

class UIWidget {
private:
	rect_t* m_bounds;
	char m_tooltip[256];

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

	void setTooltip(const char* text);

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
	void undo();
};

class UIRect : public UIWidget {
protected:
	rect_t* m_rect;

public:
	UIRect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
	~UIRect();

	void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

	virtual void draw() override;
};

class UILabel : public UIRect {
private:
	char m_text[64];
	Font* m_font;
	unsigned char m_textR, m_textG, m_textB, m_textA;

public:
	UILabel(const char* text, int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
	~UILabel();

	void setText(const char* text);
	void setFont(Font* font);
	void setTextColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

	virtual void draw() override;
};

class UIButton : public UIRect {
private:
	char m_text[64];
	rect_t* m_overlay;
	std::function<void(UIButton*)> m_clickFunc;

public:
	UIButton(const char* text, int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
	~UIButton();

	void setText(const char* text);
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
	void setValue(unsigned char value);
	unsigned char getValue();

	virtual void update() override;
	virtual void draw() override;

private:
	void refreshValue();
};

enum UIEditBitmapOperation {
	OPERATION_PENCIL,
	OPERATION_ERASER,
	OPERATION_LINE,
	OPERATION_EYEDROPPER,
	OPERATION_FILLBUCKET
};

class UIEditBitmap : public UIRect {
private:
	bitmap_t* m_bitmap;
	bitmap_t* m_previewBitmap;
	unsigned int m_texture;
	UIEditBitmapOperation m_selectedOp;
	unsigned char m_selectedR;
	unsigned char m_selectedG;
	unsigned char m_selectedB;
	int m_mouseXStart;
	int m_mouseYStart;
	bool m_colorChanged;
	unsigned char m_tolerance;
	unsigned char m_gridMode;

public:
	UIEditBitmap(int x, int y, int width, int height, int imageWidth, int imageHeight);
	~UIEditBitmap();

	void clear();
	void reload(int width, int height, unsigned char* data);
	void undo();

	void setDrawColor(unsigned char r, unsigned char g, unsigned char b);
	void setDrawOperation(UIEditBitmapOperation op);
	void setFillTolerance(unsigned char tolerance);
	void setGridMode(unsigned char mode);
	
	void getDrawColor(unsigned char* r, unsigned char* g, unsigned char* b);
	int getImageWidth();
	int getImageHeight();
	unsigned char* getImageData();
	bool getColorChanged();
	unsigned char getGridMode();

	virtual void update() override;
	virtual void draw() override;

private:
	void regenTexture(bool first = false);
	void updateTexture(bitmap_t* bitmap);
	void recurseFill(bitmap_t* bitmap, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char matchR, unsigned char matchG, unsigned char matchB, bool undo = true);
	void drawGrid();
	void drawPreview();
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
int uiface_get_mouse_buttons_down();
void uiface_undo();
void uiface_set_tooltip(const char* text);
void uiface_smart_color_invert(unsigned char r, unsigned char g, unsigned char b, unsigned char* out_r, unsigned char* out_g, unsigned char* out_b);

float uiface_px_size_x();
float uiface_px_size_y();
#define XNDC(x) (float(x) * uiface_px_size_x() - 1.0f)
#define YNDC(x) (float(x) * -uiface_px_size_y() + 1.0f)

extern std::vector<UIWidget*> gUIWidgets;