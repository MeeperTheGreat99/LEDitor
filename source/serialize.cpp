#include "serialize.h"
#include "winapishenanigans.h"
#include <fstream>
#include <string>

#define SERIALIZE_FILE_VERSION 1

extern HWND ghWnd;

void serialize_save_image(int width, int height, unsigned char* data) {
    char filename[260];
    filename[0] = '\0';

    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "LEDitor Image Files (*.led)\0*.led\0";
    ofn.lpstrDefExt = "led";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrInitialDir = ".";
    ofn.Flags = OFN_PATHMUSTEXIST;
    if (!GetSaveFileNameA(&ofn)) {
        return;
    }

	std::ofstream file(ofn.lpstrFile, std::ios::binary);
	if (!file.is_open()) {
		MessageBoxA(nullptr, "Can't write to that file.", "Joyous occasion", MB_OK | MB_ICONERROR);
		return;
	}

    int version = SERIALIZE_FILE_VERSION;
    file.write("led ", 4);
    file.write("ver ", 4);
    file.write((char*)&version, sizeof(int));
	file.write((char*)&width, sizeof(int));
	file.write((char*)&height, sizeof(int));
	file.write((char*)data, width * height * 3);

    if (file.fail()) {
        MessageBoxA(nullptr, "An error occured while writing to that file.", "Joyous occasion", MB_OK | MB_ICONERROR);
		return;
    }

	file.close();

	MessageBoxA(nullptr, "File saved successfully.", "Info", MB_OK | MB_ICONINFORMATION);
}

void serialize_load_image(int* width, int* height, unsigned char** data) {
    char filename[260];
    filename[0] = '\0';

    *data = nullptr;

    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "LEDitor Image Files (*.led)\0*.led\0";
    ofn.lpstrDefExt = "led";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrInitialDir = ".";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (!GetOpenFileNameA(&ofn))
        return;

	std::ifstream file(ofn.lpstrFile, std::ios::binary);
	if (!file.is_open()) {
		MessageBoxA(nullptr, "Can't open that file.", "Joyous occasion", MB_OK | MB_ICONERROR);
		return;
	}

    char header[4];
    file.read(header, 4);
    if (header[0] != 'l' || header[1] != 'e' || header[2] != 'd' || header[3] != ' ') {
        MessageBoxA(nullptr, "That is not a valid LEDitor image file.", "Joyous occasion", MB_OK | MB_ICONERROR);
        return;
    }

    int version = 1;

    char versionheader[4];
    file.read(versionheader, 4);
    if (versionheader[0] != 'v' || versionheader[1] != 'e' || versionheader[2] != 'r' || versionheader[3] != ' ') {
        MessageBoxA(nullptr, "That file has an outdated format; please save it again when possible.", "Info", MB_OK | MB_ICONINFORMATION);
        version = 0;
        file.seekg(-4, std::ios::cur);
    }

    if (version > 0)
        file.read((char*)&version, sizeof(int));

	file.read((char*)width, sizeof(int));
	file.read((char*)height, sizeof(int));

    unsigned char* image = new unsigned char[*width * *height * 3];
	file.read((char*)image, *width * *height * 3);

    if (file.fail()) {
        MessageBoxA(nullptr, "An error occured while reading from that file.", "Joyous occasion", MB_OK | MB_ICONERROR);
        delete[] image;
		return;
    }

    *data = image;

	file.close();
}

void serialize_export_array1d(int width, int height, unsigned char* data) {
    std::string str = "int image[] = {\n";
    bool reverse = true;
    for (int y = 0; y < height; y++) {
        str += "    ";
        if (reverse) {
            for (int x = width; x--;) {
                int idx = (y * width + x) * 3;
                str += std::to_string(data[idx]) + ", ";
                str += std::to_string(data[idx + 1]) + ", ";
                str += std::to_string(data[idx + 2]) + ", ";
            }
        } else {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 3;
                str += std::to_string(data[idx]) + ", ";
                str += std::to_string(data[idx + 1]) + ", ";
                str += std::to_string(data[idx + 2]) + ", ";
            }
        }
        str += '\n';
        reverse = !reverse;
    }

    str.pop_back();
    str.pop_back();
    str.pop_back();
    str += "\n};";

    OpenClipboard(ghWnd);
    EmptyClipboard();
    HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE, str.length() + 1);
    HANDLE hMemCpy = GlobalLock(hMem);
    memcpy(hMemCpy, str.c_str(), str.length() + 1);
    GlobalUnlock(hMemCpy);
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();

    MessageBoxA(nullptr, "Copied 1D Array initialization code to clipboard.", "Info", MB_OK | MB_ICONINFORMATION);
}

void serialize_export_array2d(int width, int height, unsigned char* data) {
    std::string str = "int image[][] = {\n";
    bool reverse = true;
    for (int y = 0; y < height; y++) {
        str += "    { ";
        if (reverse) {
            for (int x = width; x--;) {
                int idx = (y * width + x) * 3;
                str += std::to_string(data[idx]) + ", ";
                str += std::to_string(data[idx + 1]) + ", ";
                str += std::to_string(data[idx + 2]) + ", ";
            }
        } else {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 3;
                str += std::to_string(data[idx]) + ", ";
                str += std::to_string(data[idx + 1]) + ", ";
                str += std::to_string(data[idx + 2]) + ", ";
            }
        }
        str.pop_back();
        str.pop_back();
        str += " },\n";
        reverse = !reverse;
    }

    str.pop_back();
    str.pop_back();
    str += "\n};";

    OpenClipboard(ghWnd);
    EmptyClipboard();
    HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE, str.length() + 1);
    HANDLE hMemCpy = GlobalLock(hMem);
    memcpy(hMemCpy, str.c_str(), str.length() + 1);
    GlobalUnlock(hMemCpy);
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();

    MessageBoxA(nullptr, "Copied 2D Array initialization code to clipboard.", "Info", MB_OK | MB_ICONINFORMATION);
}