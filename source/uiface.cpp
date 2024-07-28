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
static char currentTooltip[256] = {0};

rect_t* create_rect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b) {
	rect_t* rect = new rect_t();
	rect->x = x;
	rect->y = y;
	rect->w = width;
	rect->h = height;
	rect->r = r;
	rect->g = g;
	rect->b = b;
	rect->a = 255;
	rect->texture = 0;
	return rect;
}

void destroy_rect(rect_t* rect) {
	delete rect;
}

void draw_rect(rect_t* rect) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (rect->texture) {
		glBindTexture(GL_TEXTURE_2D, rect->texture);
	}
	glBegin(GL_TRIANGLE_FAN);
	glColor4ub(rect->r, rect->g, rect->b, rect->a);
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
	}
	glDisable(GL_BLEND);
}

UIWidget::UIWidget(int x, int y, int width, int height) {
	m_bounds = create_rect(x, y, width, height, 0, 0, 0);
	m_tooltip[0] = 0;
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

void UIWidget::setTooltip(const char* text) {
	strcpy_s(m_tooltip, 256, text);
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
		if (m_tooltip[0])
			uiface_set_tooltip(m_tooltip);
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

void UIScreen::undo() {
	for (UIWidget* uiwidget : m_uiwidgets) {
		UIEditBitmap* editBitmap = dynamic_cast<UIEditBitmap*>(uiwidget);
		if (editBitmap) {
			editBitmap->undo();
		}
	}
}

UIRect::UIRect(int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b) :
UIWidget(x, y, width, height) {
	m_rect = create_rect(x, y, width, height, r, g, b);
}

UIRect::~UIRect() {
	destroy_rect(m_rect);
}

void UIRect::setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	m_rect->r = r;
	m_rect->g = g;
	m_rect->b = b;
	m_rect->a = a;
}

void UIRect::draw() {
	draw_rect(m_rect);
}

UILabel::UILabel(const char* text, int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b) :
UIRect(x, y, width, height, r, g, b) {
	setText(text);
	setFont(defaultFont);
	m_textR = m_textG = m_textB = m_textA = 255;
}

UILabel::~UILabel() {

}

void UILabel::setText(const char* text) {
	strcpy_s(m_text, 64, text);
}

void UILabel::setFont(Font* font) {
	m_font = font;
}
void UILabel::setTextColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	m_textR = r;
	m_textG = g;
	m_textB = b;
	m_textA = a;
}

void UILabel::draw() {
	UIRect::draw();

	int x = m_rect->x + m_rect->w / 2;
	int y = m_rect->y + m_rect->h / 2 - m_font->getSize() / 2;
	set_text_font(m_font);
	draw_text(m_text, x, y, true, m_textR, m_textG, m_textB);
}

UIButton::UIButton(const char* text, int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b) :
UIRect(x, y, width, height, 0, 0, 0) {
	strcpy_s(m_text, 64, text);
	m_overlay = create_rect(x, y, width, height, r, g, b);
	m_clickFunc = nullptr;
}

UIButton::~UIButton() {
	destroy_rect(m_overlay);
}

void UIButton::setText(const char* text) {
	strcpy_s(m_text, 64, text);
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
	int y = m_rect->y + m_rect->h / 2 - buttonFont->getSize() / 2;
	set_text_font(buttonFont);
	draw_text(m_text, x, y, true);
}

UISlider::UISlider(int x, int y, int width, int height) :
UIRect(x, y, width, height, 0, 0, 0) {
	m_minR = m_minG = m_minB = 0;
	m_maxR = m_maxG = m_maxB = 255;
	m_value = 255;
	m_tick = create_rect(x + width - 2, y - height / 10, 4, height * 5 / 4, 192, 192, 192);
}

UISlider::~UISlider() {
	destroy_rect(m_tick);
}

void UISlider::setMaxColor(unsigned char r, unsigned char g, unsigned char b) {
	m_maxR = r;
	m_maxG = g;
	m_maxB = b;
}

void UISlider::setValue(unsigned char value) {
	m_value = value;
	refreshValue();
}

unsigned char UISlider::getValue() {
	return m_value;
}

void UISlider::update() {
	UIRect::update();

	if (m_pressing) {
		int fractional = std::max(0, std::min(m_rect->w, mouseX - m_rect->x));
		m_value = fractional * 255 / m_rect->w;
		refreshValue();
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

	m_tick->texture = 0;
	
	if (m_hovering) {
		m_tick->r = 63;
		m_tick->g = 63;
		m_tick->b = 63;
	} else {
		m_tick->r = 0;
		m_tick->g = 0;
		m_tick->b = 0;
	}
	draw_rect(m_tick);
	m_tick->texture = buttonTexture;
	m_tick->r = 192;
	m_tick->g = 192;
	m_tick->b = 192;
	draw_rect(m_tick);
}

void UISlider::refreshValue() {
	int fractional = m_value * m_rect->w / 255;
	m_tick->x = m_rect->x + fractional - 2;
}

UIEditBitmap::UIEditBitmap(int x, int y, int width, int height, int imageWidth, int imageHeight) :
UIRect(x, y, width, height, 0, 0, 0) {
	m_bitmap = create_bitmap(imageWidth, imageHeight);
	bitmap_fill(m_bitmap, 0, 0, 0);
	m_previewBitmap = create_bitmap(imageWidth, imageHeight);
	bitmap_fill(m_previewBitmap, 0, 0, 0);

	m_texture = 0;
	regenTexture(true);
	
	m_selectedOp = OPERATION_PENCIL;
	m_selectedR = 255;
	m_selectedG = 255;
	m_selectedB = 255;
	m_colorChanged = false;
	m_tolerance = 8;
	m_gridMode = 0;
}

UIEditBitmap::~UIEditBitmap() {
	glDeleteTextures(1, &m_texture);
	destroy_bitmap(m_previewBitmap);
	destroy_bitmap(m_bitmap);
}

void UIEditBitmap::clear() {
	bitmap_fill(m_bitmap, 0, 0, 0);
	bitmap_clear_undo_blocks(m_bitmap);
}

void UIEditBitmap::reload(int width, int height, unsigned char* data) {
	bitmap_resize(m_bitmap, width, height);
	bitmap_resize(m_previewBitmap, width, height);
	memcpy(m_bitmap->image, data, width * height * 3);
}

void UIEditBitmap::undo() {
	if (!m_pressing)
		bitmap_pop_undo_block(m_bitmap);
}

void UIEditBitmap::setDrawColor(unsigned char r, unsigned char g, unsigned char b) {
	m_selectedR = r;
	m_selectedG = g;
	m_selectedB = b;
}

void UIEditBitmap::getDrawColor(unsigned char* r, unsigned char* g, unsigned char* b) {
	*r = m_selectedR;
	*g = m_selectedG;
	*b = m_selectedB;
}

void UIEditBitmap::setDrawOperation(UIEditBitmapOperation op) {
	m_selectedOp = op;
}

void UIEditBitmap::setFillTolerance(unsigned char tolerance) {
	m_tolerance = tolerance;
}

void UIEditBitmap::setGridMode(unsigned char mode) {
	m_gridMode = mode;
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

bool UIEditBitmap::getColorChanged() {
	if (m_colorChanged) {
		m_colorChanged = false;
		return true;
	}

	return false;
}

unsigned char UIEditBitmap::getGridMode() {
	return m_gridMode;
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

	if (m_selectedOp == OPERATION_PENCIL || m_selectedOp == OPERATION_ERASER) {
		if (m_pressed) {
			bitmap_start_undo_block(m_bitmap);
		}

		if (m_pressing && (m_pressed || xbmap != xbmapLast || ybmap != ybmapLast)) {
			if (m_selectedOp)
				bitmap_line(m_bitmap, xbmapLast, ybmapLast, xbmap, ybmap, 0, 0, 0, true);
			else
				bitmap_line(m_bitmap, xbmapLast, ybmapLast, xbmap, ybmap, m_selectedR, m_selectedG, m_selectedB, true);
		}

		if (m_released) {
			bitmap_end_undo_block(m_bitmap);
		}
	} else if (m_selectedOp == OPERATION_LINE) {
		if (m_released) {
			bitmap_start_undo_block(m_bitmap);
			bitmap_line(m_bitmap, xbmapStart, ybmapStart, xbmap, ybmap, m_selectedR, m_selectedG, m_selectedB, true);
			bitmap_end_undo_block(m_bitmap);
		}
	} else if (m_selectedOp == OPERATION_EYEDROPPER) {
		if (m_pressed) {
			int idx = (ybmap * m_bitmap->w + xbmap) * 3;
			setDrawColor(m_bitmap->image[idx], m_bitmap->image[idx + 1], m_bitmap->image[idx + 2]);
			m_colorChanged = true;
		}
	} else if (m_selectedOp == OPERATION_FILLBUCKET) {
		if (m_pressed) {
			int matchIdx = (ybmap * m_bitmap->w + xbmap) * 3;
			unsigned char matchR = m_bitmap->image[matchIdx];
			unsigned char matchG = m_bitmap->image[matchIdx + 1];
			unsigned char matchB = m_bitmap->image[matchIdx + 2];
			bitmap_start_undo_block(m_bitmap);
			recurseFill(m_bitmap, xbmap, ybmap, m_selectedR, m_selectedG, m_selectedB, matchR, matchG, matchB);
			bitmap_end_undo_block(m_bitmap);
		}
	}
}

void UIEditBitmap::draw() {
	updateTexture(m_bitmap);
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

	drawPreview();
	drawGrid();
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

void UIEditBitmap::updateTexture(bitmap_t* bitmap) {
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
	bitmap->w, bitmap->h, GL_RGB,
	GL_UNSIGNED_BYTE, bitmap->image);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void UIEditBitmap::recurseFill(bitmap_t* bitmap, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char matchR, unsigned char matchG, unsigned char matchB, bool undo) {
	int idx = (y * bitmap->w + x) * 3;
	bool bogusTest = abs((int)r - (int)matchR) <= m_tolerance &&
	abs((int)g - (int)matchG) <= m_tolerance &&
	abs((int)b - (int)matchB) <= m_tolerance;
	if (bogusTest)
		return;
	
	bitmap_pixel(bitmap, x, y, r, g, b, undo);
	for (int ofsX = -1; ofsX < 2; ofsX++) {
		for (int ofsY = -1; ofsY < 2; ofsY++) {
			if (!ofsX && !ofsY)
				continue;
			if (ofsX && ofsY)
				continue;

			int testx = std::max(0, std::min(bitmap->w - 1, x + ofsX));
			int testy = std::max(0, std::min(bitmap->h - 1, y + ofsY));
			if (testx == x && testy == y)
				continue;

			idx = (testy * bitmap->w + testx) * 3;
			bool test = abs((int)bitmap->image[idx] - (int)matchR) <= m_tolerance &&
			abs((int)bitmap->image[idx + 1] - (int)matchG) <= m_tolerance &&
			abs((int)bitmap->image[idx + 2] - (int)matchB) <= m_tolerance;
			if (test) {
				recurseFill(bitmap, testx, testy, r, g, b, matchR, matchG, matchB, undo);
			}
		}
	}
}

void UIEditBitmap::drawGrid() {
	if (m_gridMode == 1) {
		glEnable(GL_LINE_SMOOTH);
		glBegin(GL_LINES);

		for (int x = 1; x < m_bitmap->w; x++) {
			/*
			int y;
			int r = 0, g = 0, b = 0;
			for (y = 1; y < m_bitmap->h; y++) {
				int idx = (y * m_bitmap->w + x) * 3;
				r += m_bitmap->image[idx];
				g += m_bitmap->image[idx + 1];
				b += m_bitmap->image[idx + 2];
			}
			r /= y;
			g /= y;
			b /= y;
			unsigned char invertR, invertG, invertB;
			uiface_smart_color_invert(r, g, b, &invertR, &invertG, &invertB);
			glColor3ub(invertR, invertG, invertB);
			*/

			glColor3ub(255, 255, 255);

			int xsize = x * m_rect->w / (m_bitmap->w);
			glVertex2f(XNDC(m_rect->x + xsize), YNDC(m_rect->y));
			glVertex2f(XNDC(m_rect->x + xsize), YNDC(m_rect->y + m_rect->h));
		}
		for (int y = 1; y < m_bitmap->h; y++) {
			/*
			int x;
			int r = 0, g = 0, b = 0;
			for (x = 1; x < m_bitmap->w; x++) {
				int idx = (y * m_bitmap->w + x) * 3;
				r += m_bitmap->image[idx];
				g += m_bitmap->image[idx + 1];
				b += m_bitmap->image[idx + 2];
			}
			r /= y;
			g /= y;
			b /= y;
			unsigned char invertR, invertG, invertB;
			uiface_smart_color_invert(r, g, b, &invertR, &invertG, &invertB);
			glColor3ub(invertR, invertG, invertB);
			*/

			glColor3ub(255, 255, 255);

			int ysize = y * m_rect->h / (m_bitmap->h);
			glVertex2f(XNDC(m_rect->x), YNDC(m_rect->y + ysize));
			glVertex2f(XNDC(m_rect->x + m_rect->w), YNDC(m_rect->y + ysize));
		}

		glEnd();
		glDisable(GL_LINE_SMOOTH);
	} else if (m_gridMode == 2) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_POINTS);
		glColor4ub(255, 255, 255, 127);

		for (int x = 0; x < m_bitmap->w + 1; x++) {
			for (int y = 0; y <  m_bitmap->h + 1; y++) {
				int xsize = x * m_rect->w / (m_bitmap->w);
				int ysize = y * m_rect->h / (m_bitmap->h);
				glVertex2f(XNDC(m_rect->x + xsize), YNDC(m_rect->y + ysize));
			}
		}

		glEnd();
		glDisable(GL_BLEND);
	}
}

void UIEditBitmap::drawPreview() {
	int xbmap = (mouseX - m_rect->x) * m_bitmap->w / m_rect->w;
	int ybmap = (mouseY - m_rect->y) * m_bitmap->h / m_rect->h;
	int xbmapLast = (mouseXLast - m_rect->x) * m_bitmap->w / m_rect->w;
	int ybmapLast = (mouseYLast - m_rect->y) * m_bitmap->h / m_rect->h;
	int xbmapStart = (m_mouseXStart - m_rect->x) * m_bitmap->w / m_rect->w;
	int ybmapStart = (m_mouseYStart - m_rect->y) * m_bitmap->h / m_rect->h;

	if ((m_pressing && m_selectedOp == OPERATION_LINE) || (m_hovering && m_selectedOp == OPERATION_FILLBUCKET)) {
		memcpy(m_previewBitmap->image, m_bitmap->image, m_previewBitmap->w * m_previewBitmap->h * 3);

		if (m_selectedOp == OPERATION_LINE)
			bitmap_line(m_previewBitmap, xbmapStart, ybmapStart, xbmap, ybmap, m_selectedR, m_selectedG, m_selectedB);
		else {
			int idx = (ybmap * m_bitmap->w + xbmap) * 3;
			unsigned char r = m_bitmap->image[idx];
			unsigned char g = m_bitmap->image[idx + 1];
			unsigned char b = m_bitmap->image[idx + 2];
			recurseFill(m_previewBitmap, xbmap, ybmap, m_selectedR, m_selectedG, m_selectedB, r, g, b, false);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		updateTexture(m_previewBitmap);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glBegin(GL_TRIANGLE_FAN);
		glColor4ub(255, 255, 255, 127);

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
		glDisable(GL_BLEND);
	} else if (m_hovering) {
		int xsize = m_rect->w / (m_bitmap->w - 1);
		int ysize = m_rect->h / (m_bitmap->h - 1);
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
	uiface_set_tooltip("");
	currentScreen->update();

	mouseXLast = mouseX;
	mouseYLast = mouseY;
	mouseButtonsLast = mouseButtons;
}

void uiface_draw() {
	currentScreen->draw();
	if (currentTooltip[0]) {
		set_text_font(defaultFont);
		int width = get_text_width_max(currentTooltip);
		rect_t textRect;
		textRect.r = 0;
		textRect.g = 0;
		textRect.b = 0;
		textRect.a = 127;
		textRect.texture = 0;

		textRect.x = mouseX + 16;
		textRect.y = mouseY + 16;
		textRect.w = width + 16;
		textRect.h = get_text_height(currentTooltip) + 16;
		if (textRect.x + textRect.w > screenW)
			textRect.x -= textRect.x + textRect.w - screenW;
		if (textRect.y + textRect.h > screenH)
			textRect.y -= textRect.y + textRect.h - screenH;

		draw_rect(&textRect);
		draw_text(currentTooltip, textRect.x + 8, textRect.y + 8);
	}
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

int uiface_get_mouse_buttons_down() {
	return mouseButtons;
}

void uiface_undo() {
	currentScreen->undo();
}

void uiface_set_tooltip(const char* text) {
	strcpy_s(currentTooltip, 256, text);
}

void uiface_smart_color_invert(unsigned char r, unsigned char g, unsigned char b, unsigned char* out_r, unsigned char* out_g, unsigned char* out_b) {
	int rdist = abs(127 - (int)r);
	int gdist = abs(127 - (int)g);
	int bdist = abs(127 - (int)b);
	unsigned char illiteracy = (unsigned char)((rdist + gdist + bdist) * 255 / 3);
	if (illiteracy > 31 && (rdist < 63 && gdist < 63)) {
		*out_r = 0;
		*out_g = 0;
		*out_b = 0;
	} else {
		*out_r = 255 - r;
		*out_g = 255 - g;
		*out_b = 255 - b;
	}
}

float uiface_px_size_x() {
	return pxSizeX;
}

float uiface_px_size_y() {
	return pxSizeY;
}