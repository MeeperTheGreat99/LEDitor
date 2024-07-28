#include "bitmap.h"
#include <cmath>

bitmap_undo_op_t* create_undo_op(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
	bitmap_undo_op_t* undo_op = new bitmap_undo_op_t();
	undo_op->x = x;
	undo_op->y = y;
	undo_op->r = r;
	undo_op->g = g;
	undo_op->b = b;
	return undo_op;
}

void destroy_undo_op(bitmap_undo_op_t* undo_op) {
	delete undo_op;
}

bitmap_undo_block_t* create_undo_block() {
	bitmap_undo_block_t* undo_block = new bitmap_undo_block_t();
	undo_block->undo_ops = {};
	return undo_block;
}

void destroy_undo_block(bitmap_undo_block_t* undo_block) {
	for (bitmap_undo_op_t* undo_op : undo_block->undo_ops)
		destroy_undo_op(undo_op);
	undo_block->undo_ops.clear();

	delete undo_block;
}

bitmap_t* create_bitmap(int width, int height) {
	bitmap_t* bitmap = new bitmap_t();
	bitmap->w = width;
	bitmap->h = height;
	bitmap->image = new unsigned char[width * height * 3];
	bitmap->cur_undo_block = nullptr;
	bitmap->undo_blocks = {};
	return bitmap;
}

void destroy_bitmap(bitmap_t* bitmap) {
	for (bitmap_undo_block_t* undo_block : bitmap->undo_blocks)
		destroy_undo_block(undo_block);
	bitmap->undo_blocks.clear();

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

void bitmap_pixel(bitmap_t* bitmap, int x, int y, unsigned char r, unsigned char g, unsigned char b, bool undo) {
	if (x < 0 || x >= bitmap->w || y < 0 || y >= bitmap->h)
		return;

	int idx = (y * bitmap->w + x) * 3;
	if (undo)
		bitmap_push_undo_op(bitmap, x, y, bitmap->image[idx], bitmap->image[idx + 1], bitmap->image[idx + 2]);
	bitmap->image[idx] = r;
	bitmap->image[idx + 1] = g;
	bitmap->image[idx + 2] = b;
}

void bitmap_line(bitmap_t* bitmap, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b, bool undo) {
	int dx = abs(x2 - x1);
	int dy = -abs(y2 - y1);
	int sx = x1 < x2 ? 1 : -1;
	int sy = y1 < y2 ? 1 : -1;
	int err1 = dx + dy;
	int err2;

	bitmap_pixel(bitmap, x1, y1, r, g, b, undo);
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
		bitmap_pixel(bitmap, x1, y1, r, g, b, undo);
	}
}

void bitmap_start_undo_block(bitmap_t* bitmap) {
	bitmap->cur_undo_block = create_undo_block();
}

void bitmap_end_undo_block(bitmap_t* bitmap) {
	bitmap->undo_blocks.push_back(bitmap->cur_undo_block);
	bitmap->cur_undo_block = nullptr;
}

void bitmap_push_undo_op(bitmap_t* bitmap, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
	bitmap->cur_undo_block->undo_ops.push_back(create_undo_op(x, y, r, g, b));
}

void bitmap_pop_undo_block(bitmap_t* bitmap) {
	size_t blocksize = bitmap->undo_blocks.size();
	if (blocksize < 1)
		return;
		
	bitmap_undo_block_t* undo_block = bitmap->undo_blocks[blocksize - 1];

	size_t opsize = undo_block->undo_ops.size();
	for (int i = 0; i < opsize; i++) {
		bitmap_pop_undo_op(bitmap);
	}

	destroy_undo_block(undo_block);
	bitmap->undo_blocks.pop_back();
}

void bitmap_pop_undo_op(bitmap_t* bitmap) {
	size_t blocksize = bitmap->undo_blocks.size();
	if (blocksize < 1)
		return;

	bitmap_undo_block_t* undo_block = bitmap->undo_blocks[blocksize - 1];

	size_t opsize = undo_block->undo_ops.size();
	if (opsize < 1)
		return;

	bitmap_undo_op_t* undo_op = undo_block->undo_ops[opsize - 1];
	bitmap_pixel(bitmap, undo_op->x, undo_op->y, undo_op->r, undo_op->g, undo_op->b);
	destroy_undo_op(undo_op);
	undo_block->undo_ops.pop_back();
}

void bitmap_clear_undo_blocks(bitmap_t* bitmap) {
	for (bitmap_undo_block_t* undo_block : bitmap->undo_blocks)
		destroy_undo_block(undo_block);
	bitmap->undo_blocks.clear();
}