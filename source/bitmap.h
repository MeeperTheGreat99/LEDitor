#pragma once
#include <vector>

typedef struct bitmap_undo_op_s {
	int x, y;
	int r, g, b;
} bitmap_undo_op_t;

bitmap_undo_op_t* create_undo_op(int x, int y, unsigned char r, unsigned char g, unsigned char b);
void destroy_undo_op(bitmap_undo_op_t* undo_op);

typedef struct bitmap_undo_block_s {
	std::vector<bitmap_undo_op_t*> undo_ops;
} bitmap_undo_block_t;

bitmap_undo_block_t* create_undo_block();
void destroy_undo_block(bitmap_undo_block_t* undo_block);

typedef struct bitmap_s {
	int w, h;
	unsigned char* image;
	bitmap_undo_block_t* cur_undo_block;
	std::vector<bitmap_undo_block_t*> undo_blocks;
} bitmap_t;

bitmap_t* create_bitmap(int width, int height);
void destroy_bitmap(bitmap_t* bitmap);
void bitmap_resize(bitmap_t* bitmap, int width, int height);
void bitmap_fill(bitmap_t* bitmap, unsigned char r, unsigned char g, unsigned char b);
void bitmap_pixel(bitmap_t* bitmap, int x, int y, unsigned char r, unsigned char g, unsigned char b, bool undo = false);
void bitmap_line(bitmap_t* bitmap, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b, bool undo = false);

void bitmap_start_undo_block(bitmap_t* bitmap);
void bitmap_end_undo_block(bitmap_t* bitmap);
void bitmap_push_undo_op(bitmap_t* bitmap, int x, int y, unsigned char r, unsigned char g, unsigned char b);
void bitmap_pop_undo_block(bitmap_t* bitmap);
void bitmap_pop_undo_op(bitmap_t* bitmap);
void bitmap_clear_undo_blocks(bitmap_t* bitmap);