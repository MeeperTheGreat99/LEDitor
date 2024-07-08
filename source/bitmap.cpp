#include "bitmap.h"
#include <cmath>

bitmap_t* create_bitmap(int width, int height) {
	bitmap_t* bitmap = new bitmap_t();
	bitmap->w = width;
	bitmap->h = height;
	bitmap->image = new unsigned char[width * height * 3];
	return bitmap;
}

void destroy_bitmap(bitmap_t* bitmap) {
	delete[] bitmap->image;
	delete bitmap;
}

void bitmap_resize(bitmap_t* bitmap, int width, int height) {
	delete[] bitmap->image;
	bitmap->image = new unsigned char[width * height * 3];
}

void bitmap_fill(bitmap_t* bitmap, unsigned char r, unsigned char g, unsigned char b) {
	unsigned char* image = bitmap->image;
	for (int i = 0; i < bitmap->w * bitmap->h; i++) {
		int idx = i * 3;
		image[idx] = r;
		image[idx + 1] = g;
		image[idx + 2] = b;
	}
}

void bitmap_pixel(bitmap_t* bitmap, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
	if (x < 0 || x >= bitmap->w || y < 0 || y >= bitmap->h)
		return;

	int idx = (y * bitmap->w + x) * 3;
	bitmap->image[idx] = r;
	bitmap->image[idx + 1] = g;
	bitmap->image[idx + 2] = b;
}

void bitmap_line(bitmap_t* bitmap, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b) {
	int dx = abs(x2 - x1);
	int dy = -abs(y2 - y1);
	int sx = x1 < x2 ? 1 : -1;
	int sy = y1 < y2 ? 1 : -1;
	int err1 = dx + dy;
	int err2;

	bitmap_pixel(bitmap, x1, y1, r, g, b);
	while (x1 != x2 || y1 != y2) {
		err2 = 2 * err1;
		if (err2 >= dy) {
			err1 += dy;
			x1 += sx;
		}
		if (err2 <= dx) {
			err1 += dx;
			y1 += sy;
		}
		bitmap_pixel(bitmap, x1, y1, r, g, b);
	}
}