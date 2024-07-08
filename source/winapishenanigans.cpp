#include "winapishenanigans.h"
#include "display.h"
#include "uiface.h"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool quitProgram = false;
extern HWND ghWnd;
extern HINSTANCE ghInstance;

void winapi_initialize() {
	WNDCLASSEXA wc = {0};
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = "LEDitorWC";
	wc.hInstance = ghInstance;
	wc.lpfnWndProc = &WindowProc;
	wc.hCursor = LoadCursorA(NULL, (const char*)IDC_ARROW);
	wc.style = CS_OWNDC;
	RegisterClassExA(&wc);

	// adjust actual window size so the client area size is exactly as desired
	RECT windowrect = {0, 0, 640, 480};
	DWORD style = WS_OVERLAPPEDWINDOW;
	AdjustWindowRectEx(&windowrect, style, FALSE, 0);

	ghWnd = CreateWindowExA(
		0, "LEDitorWC", "LEDitor", style & ~WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, windowrect.right - windowrect.left, windowrect.bottom - windowrect.top,
		NULL, NULL, ghInstance, NULL
	);

	// horrendous way to check windows version info since microsoft broke the
	// easy way out of spite; worth it for dark title bar though :)
	HMODULE hNtdll = LoadLibraryA("Ntdll.dll");
	DWORD (*RtlGetVersion)(PRTL_OSVERSIONINFOEXW) = nullptr;
	if (hNtdll)
		RtlGetVersion = (DWORD (*)(PRTL_OSVERSIONINFOEXW))GetProcAddress(hNtdll, "RtlGetVersion");
	OSVERSIONINFOEXA osinfo = {0};
	osinfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
	if (RtlGetVersion && !RtlGetVersion((PRTL_OSVERSIONINFOEXW)&osinfo) && osinfo.dwMajorVersion >= 10) {
		// backwards compatibility since update 20H1 changed the dark mode
		// attribute index from 19 to 20 for some reason
		BOOL darkMode = true;
		DwmSetWindowAttribute(ghWnd, osinfo.dwBuildNumber >= 19041 ? 20 : 19, &darkMode, sizeof(darkMode));
	}
	if (hNtdll)
		FreeLibrary(hNtdll);
}

void winapi_shutdown() {
	DestroyWindow(ghWnd);
	UnregisterClassA("LEDitorWC", ghInstance);
}

void winapi_show() {
	ShowWindow(ghWnd, SW_SHOW);
}

bool winapi_run() {
	MSG msg = {0};
	while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	return !quitProgram;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CLOSE:
			quitProgram = true;
			return 0;

		case WM_ERASEBKGND:
			return 1;

		case WM_SIZE: {
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			display_resize(width, height);
			uiface_resize(width, height);
			return 0;
		}

		case WM_MOUSEMOVE:
			uiface_mouse_position(LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_LBUTTONDOWN:
			uiface_mouse_buttons_down(MOUSE_LMB);
			return 0;
		case WM_RBUTTONDOWN:
			uiface_mouse_buttons_down(MOUSE_RMB);
			return 0;
		case WM_MBUTTONDOWN:
			uiface_mouse_buttons_down(MOUSE_MMB);
			return 0;
		case WM_XBUTTONDOWN:
			if (HIWORD(wParam) == XBUTTON1)
				uiface_mouse_buttons_down(MOUSE_XMB1);
			else
				uiface_mouse_buttons_down(MOUSE_XMB2);
			return 0;

		case WM_LBUTTONUP:
			uiface_mouse_buttons_up(MOUSE_LMB);
			return 0;
		case WM_RBUTTONUP:
			uiface_mouse_buttons_up(MOUSE_RMB);
			return 0;
		case WM_MBUTTONUP:
			uiface_mouse_buttons_up(MOUSE_MMB);
			return 0;
		case WM_XBUTTONUP:
			if (HIWORD(wParam) == XBUTTON1)
				uiface_mouse_buttons_up(MOUSE_XMB1);
			else
				uiface_mouse_buttons_up(MOUSE_XMB2);
			return 0;
	}

	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}