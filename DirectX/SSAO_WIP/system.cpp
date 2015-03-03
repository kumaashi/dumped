#include "if.h"

//------------------------------------------------------------------------------
// System
//------------------------------------------------------------------------------

LRESULT WINAPI System::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int temp = wParam & 0xFFF0;
	switch(msg)
	{
	case WM_SYSCOMMAND:
		if(temp == SC_MONITORPOWER || temp == SC_SCREENSAVE)
		{
			return 0;
		}
		break;
	case WM_IME_SETCONTEXT:
		lParam &= ~ISC_SHOWUIALL;
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	case WM_KEYDOWN:
		if(wParam == VK_ESCAPE) PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void System::Init(char *AppName, int w, int h, BOOL IsWindow)
{
	Width  = w;
	Height = h;

	RECT   rc;
	DWORD  Style = IsWindow ? WS_OVERLAPPEDWINDOW : WS_POPUP;
	DWORD  StyleEx = WS_EX_APPWINDOW | (IsWindow ? WS_EX_WINDOWEDGE : 0);

	static WNDCLASSEX wcex = {
		sizeof(WNDCLASSEX), CS_CLASSDC | CS_VREDRAW | CS_HREDRAW,
		WindowProc, 0L, 0L, NULL, LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH),
		NULL, AppName, NULL
	};

	SetRect(&rc, 0, 0, Width, Height);
	AdjustWindowRectEx(&rc, Style, FALSE, StyleEx);
	rc.right -= rc.left;
	rc.bottom -= rc.top;

	wcex.hInstance = GetModuleHandle(NULL);
	if (!RegisterClassEx(&wcex)) return;
	hWnd = CreateWindowEx(
	           StyleEx, AppName, AppName, Style,
	           //(GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2,
	           //(GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2,
							0,0,
	           rc.right, rc.bottom, NULL, NULL, wcex.hInstance, NULL);
	if (!hWnd) return;
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	IsWindow ? (void)0 : ShowCursor(FALSE);

	return;
}

bool System::IsDone() {
	MSG	msg;
	bool bDone = false;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			bDone = true;
			break;
		} else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return bDone;
}
