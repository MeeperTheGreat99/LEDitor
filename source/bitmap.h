#pragma once

typedef struct bitmap_s {
	int w, h;
	unsigned char* image;
} bitmap_t;

bitmap_t* create_bitmap(int width, int height);
void destroy_bitmap(bitmap_t* bitmap);
void bitmap_resize(bitmap_t* bitmap, int width, int height);
void bitmap_fill(bitmap_t* bitmap, unsigned char r, unsigned char g, unsigned char b);
void bitmap_pixel(bitmap_t* bitmap, int x, int y, unsigned char r, unsigned char g, unsigned char b);
void bitmap_line(bitmap_t* bitmap, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b);