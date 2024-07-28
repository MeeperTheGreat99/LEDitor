#include "winapishenanigans.h"
#include "display.h"
#include "uiface.h"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool quitProgram = false;
extern int mainWidth;
extern int mainHeight;
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
	RECT windowrect = {0, 0, mainWidth, mainHeight};
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
	TRACKMOUSEEVENT trackMouse = {0};
	trackMouse.cbSize = sizeof(TRACKMOUSEEVENT);
	trackMouse.dwFlags = TME_LEAVE;
	trackMouse.hwndTrack = ghWnd;
	trackMouse.dwHoverTime = HOVER_DEFAULT;

	switch (uMsg) {
		case WM_CLOSE:
			quitProgram = true;
			return 0;

		case WM_ERASEBKGND:
			return 1;

		case WM_SIZE:
			mainWidth = LOWORD(lParam);
			mainHeight = HIWORD(lParam);
			display_resize(mainWidth, mainHeight);
			uiface_resize(mainWidth, mainHeight);
			return 0;

		case WM_MOUSEMOVE: {
			int x = (int)LOWORD(lParam);
			int y = (int)HIWORD(lParam);
			if (x > 32767)
				x -= 65536;
			if (y > 32767)
				y -= 65536;
			uiface_mouse_position(x, y);
			TrackMouseEvent(&trackMouse);
			return 0;
		}

		case WM_LBUTTONDOWN:
			SetCapture(ghWnd);
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
			ReleaseCapture();
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

		case WM_KEYDOWN:
			if (wParam == 'Z' && GetKeyState(VK_CONTROL))
				uiface_undo();
	}

	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}