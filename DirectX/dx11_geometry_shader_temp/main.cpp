//------------------------------------------------------------------------------
//
//  main.cpp
//
//------------------------------------------------------------------------------
#include "common.h"

//------------------------------------------------------------------------------
//  define function
//------------------------------------------------------------------------------
static LRESULT WINAPI WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//------------------------------------------------------------------------------
//  variable
//------------------------------------------------------------------------------
//system
int      argc = 0;
char   **argv = NULL;

//Window
LPSTR    AppName = "test";
HWND     hWnd = NULL;

//------------------------------------------------------------------------------
//  table
//------------------------------------------------------------------------------
//Window Class Ex
static WNDCLASSEX wcex = {
  sizeof(WNDCLASSEX), CS_CLASSDC | CS_VREDRAW | CS_HREDRAW,
  WindowProc, 0L, 0L, NULL, LoadIcon(NULL, IDI_APPLICATION),
  LoadCursor(NULL, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH),
  NULL, AppName, NULL
};



//------------------------------------------------------------------------------
//
//  func
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  WindowProc
//------------------------------------------------------------------------------
static LRESULT WINAPI WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  int temp = wParam & 0xFFF0;
  switch(msg) {
  case WM_SYSCOMMAND:
    if(temp == SC_MONITORPOWER || temp == SC_SCREENSAVE) {
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


//------------------------------------------------------------------------------
//  entry point
//------------------------------------------------------------------------------
int main(int a, char *b[]) {
  MSG  msg;
  argc = a;
  argv = b;
  BOOL Windowed = TRUE;
  
  
  //Create Window
  wcex.hInstance = GetModuleHandle(NULL);
  if(!RegisterClassEx(&wcex)) return 1;
  hWnd = CreateWindowEx(WS_EX_APPWINDOW | (Windowed ? WS_EX_WINDOWEDGE : 0),
    AppName, AppName, Windowed  ? WS_OVERLAPPEDWINDOW : WS_POPUP,
    0, 0, ScreenX, ScreenY, NULL, NULL, wcex.hInstance, NULL);
  if(!hWnd) {
    OutputDebugString("ERROR : Can't Create Window");
    return 0;
  }
  ShowWindow(hWnd, SW_SHOW);
  UpdateWindow(hWnd);
  Windowed ? (void)0 : ShowCursor(FALSE);
  
  if(InitDevice(hWnd, ScreenX, ScreenY) == S_OK){
    BOOL bDone = FALSE;
    InitScene();
    while(!bDone) {
      while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        if(msg.message == WM_QUIT) {
          bDone = TRUE;
          break;
        } else {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }
      DoScene();
      if(Windowed) Sleep(SLEEP_60HZ);
    }
  }
  
  //Terminate
  TermScene();
  TermDevice();
  return 0;
}

