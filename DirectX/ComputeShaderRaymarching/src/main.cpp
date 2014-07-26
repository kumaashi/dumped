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

//fps : http://www.t-pot.com/program/13_fps/index.html
void show_fps() {
  static DWORD    last = timeGetTime();
  static DWORD    frames = 0;
  static char     buf[256] = "";
  DWORD           current;
  current = timeGetTime();
  frames++;
  if(1000 <= current - last) {
      float dt = (float)(current - last) / 1000.0f;
      float fps = (float)frames / dt;
      last = current;
      frames = 0;
      sprintf(buf, "%.02f fps            ", fps);
      printf("%s\n", buf);
  }
}

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
  
  RECT  rc;
  DWORD Style   =  Windowed  ? WS_OVERLAPPEDWINDOW : WS_POPUP;
  DWORD StyleEx = WS_EX_APPWINDOW | (Windowed ? WS_EX_WINDOWEDGE : 0);
  SetRect( &rc, 0, 0, WindowX, WindowY );
  AdjustWindowRectEx( &rc, Style, FALSE, StyleEx);
  rc.right  -= rc.left;
  rc.bottom -= rc.top;
  
  //Create Window
  wcex.hInstance = GetModuleHandle(NULL);
  if(!RegisterClassEx(&wcex)) return 1;
  hWnd = CreateWindowEx(StyleEx,
    AppName, AppName, Style,
    (GetSystemMetrics( SM_CXSCREEN ) - rc.right)  / 2,
    (GetSystemMetrics( SM_CYSCREEN ) - rc.bottom) / 2,
    rc.right, rc.bottom, NULL, NULL, wcex.hInstance, NULL);
  if(!hWnd) {
    OutputDebugString("ERROR : Can't Create Window");
    return 0;
  }
  ShowWindow(hWnd, SW_SHOW);
  UpdateWindow(hWnd);
  Windowed ? (void)0 : ShowCursor(FALSE);
  
  if(D3DInitDevice(hWnd, ScreenX, ScreenY, "main.fx") == S_OK){
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
      //if(Windowed) Sleep(SLEEP_60HZ);
    }
  }
  
  //Terminate
  TermScene();
  D3DTermDevice();
  return 0;
}

