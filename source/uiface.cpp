#include "uiface.h"
#include "display.h"
#include "text.h"
#include <algorithm>

std::vector<UIWidget*> gUIWidgets = {};

static int screenW = 0;
static int screenH = 0;
static int mouseX = 0;
static int mouseY = 0;
static int mouseXLast = 0;
static int mouseYLast = 0;
static int mouseButtons = 0;
static int mouseButtonsLast = 0;
static float pxSizeX = 0.0f;
static float pxSizeY = 0.0f;
static unsigned int buttonTexture = 0;
static UIScreen* currentScreen = nullptr;

rect_t* create_rect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b) {
	rect_t* rect = new rect_t();
	rect->x = x;
	rect->y = y;
	rect->w = width;
	rect->h = height;
	rect->r = r;
	rect->g = g;
	rect->b = b;
	rect->texture = 0;
	return rect;
}

void destroy_rect(rect_t* rect) {
	delete rect;
}

void draw_rect(rect_t* rect) {
	if (rect->texture) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, rect->texture);
	}
	glBegin(GL_TRIANGLE_FAN);
	glColor3ub(rect->r, rect->g, rect->b);
	glTexCoord2i(0, 1);
	glVertex2f(XNDC(rect->x), YNDC(rect->y));
	glTexCoord2i(0, 0);
	glVertex2f(XNDC(rect->x), YNDC(rect->y + rect->h));
	glTexCoord2i(1, 0);
	glVertex2f(XNDC(rect->x + rect->w), YNDC(rect->y + rect->h));
	glTexCoord2i(1, 1);
	glVertex2f(XNDC(rect->x + rect->w), YNDC(rect->y));
	glEnd();
	if (rect->texture) {
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_BLEND);
	}
}

UIWidget::UIWidget(int x, int y, int width, int height) {
	m_bounds = create_rect(x, y, width, height, 0, 0, 0);
	m_hovering = false;
	m_pressing = false;
	m_hoveringLast = false;
	m_pressingLast = false;
	m_hovered = false;
	m_unhovered = false;
	m_pressed = false;
	m_released = false;

	gUIWidgets.push_back(this);
}

UIWidget::~UIWidget() {
	destroy_rect(m_bounds);
}

void UIWidget::update() {
	int x1 = m_bounds->x, y1 = m_bounds->y;
	int x2 = x1 + m_bounds->w, y2 = y1 + m_bounds->h;

	m_hovered = false;
	m_unhovered = false;
	if (mouseX > x1 && mouseX < x2 && mouseY > y1 && mouseY < y2) {
		m_hovering = true;
		if (!m_hoveringLast)
			m_hovered = true;
	} else {
		m_hovering = false;
		if (m_hoveringLast)
			m_unhovered = true;
	}

	m_pressed = false;
	m_released = false;
	if (mouseButtons & MOUSE_LMB) {
		if (!m_pressing && !(mouseButtonsLast & MOUSE_LMB) && m_hovering) {
			m_pressing = true;
			if (!m_pressingLast)
				m_pressed = true;
		}
	} else {
		m_pressing = false;
		if (m_pressingLast)
			m_released = true;
	}

	m_hoveringLast = m_hovering;
	m_pressingLast = m_pressing;
}

UIScreen::UIScreen() {
	m_uiwidgets = {};
}

UIScreen::~UIScreen() {
	m_uiwidgets.clear();
}

void UIScreen::update() {
	for (UIWidget* uiwidget : m_uiwidgets)
		uiwidget->update();
}

void UIScreen::draw() {
	for (UIWidget* uiwidget : m_uiwidgets)
		uiwidget->draw();
}

void UIScreen::addUIWidget(UIWidget* uiwidget) {
	m_uiwidgets.push_back(uiwidget);
}

void UIScreen::removeUIWidget(UIWidget* uiwidget) {
	auto find = std::find(m_uiwidgets.begin(), m_uiwidgets.end(), uiwidget);
	if (find != m_uiwidgets.end())
		m_uiwidgets.erase(find);
}

UIRect::UIRect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b) :
UIWidget(x, y, width, height) {
	m_rect = create_rect(x, y, width, height, r, g, b);
}

UIRect::~UIRect() {
	destroy_rect(m_rect);
}

void UIRect::setColor(unsigned char r, unsigned char g, unsigned char b) {
	m_rect->r = r;
	m_rect->g = g;
	m_rect->b = b;
}

void UIRect::draw() {
	draw_rect(m_rect);
}

UILabel::UILabel(const char* text, int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b) :
UIRect(x, y, width, height, r, g, b) {
	setText(text);
}

UILabel::~UILabel() {

}

void UILabel::setText(const char* text) {
	strcpy_s(m_text, 64, text);
}

void UILabel::draw() {
	UIRect::draw();
	set_text_font(defaultFont);
	draw_text(m_text, m_rect->x + m_rect->w / 2, m_rect->y + defaultFont->getSize() / 2, true);
}

UIButton::UIButton(const char* text, int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b) :
UIRect(x, y, width, height, 0, 0, 0) {
	m_text = text;
	m_overlay = create_rect(x, y, width, height, r, g, b);
	m_clickFunc = nullptr;
}

UIButton::~UIButton() {
	destroy_rect(m_overlay);
}

void UIButton::setClickFunc(std::function<void(UIButton*)> clickFunc) {
	m_clickFunc = clickFunc;
}

void UIButton::update() {
	UIRect::update();

	if (m_pressed && m_clickFunc) {
		m_clickFunc(this);
	}
}

void UIButton::draw() {
	if (!m_pressing) {
		if (m_hovering) {
			m_rect->r = 63;
			m_rect->g = 63;
			m_rect->b = 63;
		} else {
			m_rect->r = 0;
			m_rect->g = 0;
			m_rect->b = 0;
		}
		UIRect::draw();
	}

	m_overlay->texture = buttonTexture;
	draw_rect(m_overlay);
	m_overlay->texture = 0;

	int x = m_rect->x + m_rect->w / 2;
	int y = m_rect->y + buttonFont->getSize() / 2;
	set_text_font(buttonFont);
	draw_text(m_text, x, y, true);
}

UISlider::UISlider(int x, int y, int width, int height) :
UIRect(x, y, width, height, 0, 0, 0) {
	m_minR = m_minG = m_minB = 0;
	m_maxR = m_maxG = m_maxB = 255;
	m_value = 255;
	m_tick = create_rect(x + width - 4, y - height / 10, 8, height * 5 / 4, 192, 192, 192);
}

UISlider::~UISlider() {
	destroy_rect(m_tick);
}

void UISlider::setMaxColor(unsigned char r, unsigned char g, unsigned char b) {
	m_maxR = r;
	m_maxG = g;
	m_maxB = b;
}

unsigned char UISlider::getValue() {
	return m_value;
}

void UISlider::update() {
	UIRect::update();

	if (m_pressing) {
		int fractional = std::max(0, std::min(m_rect->w, mouseX - m_rect->x));
		m_value = fractional * 255 / m_rect->w;
		m_tick->x = m_rect->x + fractional - 4;
	}
}

void UISlider::draw() {
	m_rect->r = m_minR;
	m_rect->g = m_minG;
	m_rect->b = m_minB;
	UIRect::draw();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_TRIANGLE_FAN);
	glColor4ub(m_maxR, m_maxG, m_maxB, 0);
	glTexCoord2i(0, 1);
	glVertex2f(XNDC(m_rect->x), YNDC(m_rect->y));
	glTexCoord2i(0, 0);
	glVertex2f(XNDC(m_rect->x), YNDC(m_rect->y + m_rect->h));
	glColor4ub(m_maxR, m_maxG, m_maxB, 255);
	glTexCoord2i(1, 0);
	glVertex2f(XNDC(m_rect->x + m_rect->w), YNDC(m_rect->y + m_rect->h));
	glTexCoord2i(1, 1);
	glVertex2f(XNDC(m_rect->x + m_rect->w), YNDC(m_rect->y));
	glEnd();

	glDisable(GL_BLEND);

	draw_rect(m_tick);
}

UIEditBitmap::UIEditBitmap(int x, int y, int width, int height, int imageWidth, int imageHeight) :
UIRect(x, y, width, height, 0, 0, 0) {
	m_bitmap = create_bitmap(imageWidth, imageHeight);
	bitmap_fill(m_bitmap, 0, 0, 0);

	m_texture = 0;
	regenTexture(true);
	
	m_selectedOp = 0;
	m_selectedR = 255;
	m_selectedG = 255;
	m_selectedB = 255;
}

UIEditBitmap::~UIEditBitmap() {
	glDeleteTextures(1, &m_texture);
	destroy_bitmap(m_bitmap);
}

void UIEditBitmap::clear() {
	bitmap_fill(m_bitmap, 0, 0, 0);
}

void UIEditBitmap::reload(int width, int height, unsigned char* data) {
	bitmap_resize(m_bitmap, width, height);
	memcpy(m_bitmap->image, data, width * height * 3);
}

void UIEditBitmap::setDrawColor(unsigned char r, unsigned char g, unsigned char b) {
	m_selectedR = r;
	m_selectedG = g;
	m_selectedB = b;
}

void UIEditBitmap::setDrawOperation(int op) {
	m_selectedOp = op;
}

int UIEditBitmap::getImageWidth() {
	return m_bitmap->w;
}

int UIEditBitmap::getImageHeight() {
	return m_bitmap->h;
}

unsigned char* UIEditBitmap::getImageData() {
	return m_bitmap->image;
}

void UIEditBitmap::update() {
	UIRect::update();

	if (m_pressed) {
		m_mouseXStart = mouseX;
		m_mouseYStart = mouseY;
	}

	int xbmap = (mouseX - m_rect->x) * m_bitmap->w / m_rect->w;
	int ybmap = (mouseY - m_rect->y) * m_bitmap->h / m_rect->h;
	int xbmapLast = (mouseXLast - m_rect->x) * m_bitmap->w / m_rect->w;
	int ybmapLast = (mouseYLast - m_rect->y) * m_bitmap->h / m_rect->h;
	int xbmapStart = (m_mouseXStart - m_rect->x) * m_bitmap->w / m_rect->w;
	int ybmapStart = (m_mouseYStart - m_rect->y) * m_bitmap->h / m_rect->h;

	if (m_selectedOp == 0 || m_selectedOp == 1) {
		if (m_pressing) {
			if (m_selectedOp)
				bitmap_line(m_bitmap, xbmapLast, ybmapLast, xbmap, ybmap, 0, 0, 0);
			else
				bitmap_line(m_bitmap, xbmapLast, ybmapLast, xbmap, ybmap, m_selectedR, m_selectedG, m_selectedB);
		}
	} else if (m_selectedOp == 2) {
		if (m_released) {
			bitmap_line(m_bitmap, xbmapStart, ybmapStart, xbmap, ybmap, m_selectedR, m_selectedG, m_selectedB);
		}
	}
}

void UIEditBitmap::draw() {
	updateTexture();
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glBegin(GL_TRIANGLE_FAN);
	glColor3ub(255, 255, 255);

	glTexCoord2i(0, 0);
	glVertex2f(XNDC(m_rect->x), YNDC(m_rect->y));
	glTexCoord2i(0, 1);
	glVertex2f(XNDC(m_rect->x), YNDC(m_rect->y + m_rect->h));
	glTexCoord2i(1, 1);
	glVertex2f(XNDC(m_rect->x + m_rect->w), YNDC(m_rect->y + m_rect->h));
	glTexCoord2i(1, 0);
	glVertex2f(XNDC(m_rect->x + m_rect->w), YNDC(m_rect->y));

	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	if (m_hovering) {
		int xsize = m_rect->w / (m_bitmap->w - 1);
		int ysize = m_rect->h / (m_bitmap->h - 1);
		int xbmap = (mouseX - m_rect->x) * m_bitmap->w / m_rect->w;
		int ybmap = (mouseY - m_rect->y) * m_bitmap->h / m_rect->h;
		int x = m_rect->x + xbmap * m_rect->w / m_bitmap->w;
		int y = m_rect->y + ybmap * m_rect->h / m_bitmap->h;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_TRIANGLE_FAN);
		glColor4ub(m_selectedR, m_selectedG, m_selectedB, 127);

		glVertex2f(XNDC(x), YNDC(y));
		glVertex2f(XNDC(x), YNDC(y + ysize));
		glVertex2f(XNDC(x + xsize), YNDC(y + ysize));
		glVertex2f(XNDC(x + xsize), YNDC(y));

		glEnd();
		glDisable(GL_BLEND);
	}
}

void UIEditBitmap::regenTexture(bool first) {
	if (!first) {
		glDeleteTextures(1, &m_texture);
		m_texture = 0;
	}

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
	m_bitmap->w, m_bitmap->h, 0, GL_RGB,
	GL_UNSIGNED_BYTE, m_bitmap->image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void UIEditBitmap::updateTexture() {
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
	m_bitmap->w, m_bitmap->h, GL_RGB,
	GL_UNSIGNED_BYTE, m_bitmap->image);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void uiface_initialize() {
	unsigned char buttonTexturePixels[] = {
		255, 255, 255, 63, 255, 255, 255, 31,
		255, 255, 255, 255, 255, 255, 255, 192
	};

	glGenTextures(1, &buttonTexture);
	glBindTexture(GL_TEXTURE_2D, buttonTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, buttonTexturePixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void uiface_shutdown() {
	for (UIWidget* uiwidget : gUIWidgets) {
		delete uiwidget;
	}
	gUIWidgets.clear();

	glDeleteTextures(1, &buttonTexture);
}

void uiface_update() {
	currentScreen->update();

	mouseXLast = mouseX;
	mouseYLast = mouseY;
	mouseButtonsLast = mouseButtons;
}

void uiface_draw() {
	currentScreen->draw();
}

void uiface_resize(int w, int h) {
	screenW = w;
	screenH = h;
	pxSizeX = 2.0f / float(w);
	pxSizeY = 2.0f / float(h);
}

void uiface_set_screen(UIScreen* screen) {
	currentScreen = screen;
}

void uiface_add_widget(UIWidget* uiwidget) {
	gUIWidgets.push_back(uiwidget);
}

void uiface_mouse_position(int x, int y) {
	mouseX = x;
	mouseY = y;
}

void uiface_mouse_buttons_down(int buttons) {
	mouseButtons |= buttons;
}

void uiface_mouse_buttons_up(int buttons) {
	mouseButtons &= ~buttons;
}

float uiface_px_size_x() {
	return pxSizeX;
}

float uiface_px_size_y() {
	return pxSizeY;
}