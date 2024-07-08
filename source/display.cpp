#include "display.h"
#include "uiface.h"
#include "text.h"
#include <cstdio>

HDC ghDC;
HGLRC ghRC;
extern HWND ghWnd;
static int displayWidth;
static int displayHeight;

void display_initialize() {
	PIXELFORMATDESCRIPTOR pfd = {0};
	int pf;

	ghDC = GetDC(ghWnd);

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL;
	pfd.cColorBits = 32;

	pf = ChoosePixelFormat(ghDC, &pfd);
	SetPixelFormat(ghDC, pf, &pfd);

	ghRC = wglCreateContext(ghDC);
	wglMakeCurrent(ghDC, ghRC);

	glEnable(GL_TEXTURE_2D);
}

void display_shutdown() {
	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(ghRC);
	ReleaseDC(ghWnd, ghDC);
}

void display_update() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// draw background
	glBegin(GL_TRIANGLE_FAN);
	glColor3ub(255, 0, 0);
	glVertex2i(-1, 1);
	glColor3ub(127, 0, 255);
	glVertex2i(-1, -1);
	glColor3ub(0, 0, 255);
	glVertex2i(1, -1);
	glColor3ub(255, 127, 0);
	glVertex2i(1, 1);
	glEnd();

	// draw UI widgets
	uiface_draw();

	extern int majorVersion;
	extern int minorVersion;
	char label[64];
	sprintf(label, "LEDitor v%d.%d", majorVersion, minorVersion);
	set_text_font(defaultFont);
	int textWidth = get_text_width(label);
	draw_text(label, displayWidth - textWidth - 8, 8);

	SwapBuffers(ghDC);
}

void display_resize(int w, int h) {
	displayWidth = w;
	displayHeight = h;
	glViewport(0, 0, displayWidth, displayHeight);
}