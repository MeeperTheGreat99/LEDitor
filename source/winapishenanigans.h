#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>
#include <commdlg.h>

void winapi_initialize();
void winapi_shutdown();
void winapi_show();
bool winapi_run();