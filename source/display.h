#pragma once
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/wgl.h>

void display_initialize();
void display_shutdown();
void display_update();
void display_resize(int w, int h);