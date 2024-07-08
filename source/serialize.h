#pragma once

void serialize_save_image(int width, int height, unsigned char* data);
void serialize_load_image(int* width, int* height, unsigned char** data);
void serialize_export_array1d(int width, int height, unsigned char* data);
void serialize_export_array2d(int width, int height, unsigned char* data);