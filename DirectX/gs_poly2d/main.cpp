//------------------------------------------------------------------------------
//
//  main.cpp
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  include
//------------------------------------------------------------------------------
#include "common.h"

DXGI_SWAP_CHAIN_DESC    d3dsddesc;
ID3D11Device            *d3ddevice    = NULL;
ID3D11DeviceContext     *d3dcontext   = NULL;
IDXGISwapChain          *d3dswapchain = NULL;
ID3D11RenderTargetView  *d3drtv       = NULL;

//kitanai
#define D3D11CreateDeviceDefinition     &d3dsddesc, &d3dswapchain, &d3ddevice, NULL, &d3dcontext

//------------------------------------------------------------------------------
//  define function
//------------------------------------------------------------------------------
//Window Proc
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
  WindowProc, 0L, 0L, NULL, ::LoadIcon(NULL, IDI_APPLICATION),
  ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)::GetStockObject(BLACK_BRUSH),
  NULL, AppName, NULL
};

//DirectX9
DXGI_SWAP_CHAIN_DESC D3DXGetSwapChainDesc(HWND hWnd, UINT Width, UINT Height) {
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory( &sd, sizeof( sd ) );
  sd.BufferCount        = 1;
  sd.BufferDesc.Width   = Width;
  sd.BufferDesc.Height  = Height;
  sd.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferDesc.RefreshRate.Numerator   = 60;
  sd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow       = hWnd;
  sd.SampleDesc.Count   = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed           = TRUE;
  sd.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;
  sd.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  return sd;
}

//------------------------------------------------------------------------------
//
//  function definition
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
    ::PostQuitMessage(0);
    return 0;
    break;
  case WM_KEYDOWN:
    if(wParam == VK_ESCAPE) ::PostQuitMessage(0);
    break;
  }
  return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

//------------------------------------------------------------------------------
// Terminate DirectX11
//------------------------------------------------------------------------------
HRESULT TermDevice() {
  RELEASE(d3drtv       );
  RELEASE(d3dswapchain );
  RELEASE(d3dcontext   );
  RELEASE(d3ddevice    );
  
  return S_OK;
}


//------------------------------------------------------------------------------
// Initialize DirectX11
//------------------------------------------------------------------------------
HRESULT SetupDevice(HWND hWnd, UINT width ,UINT height) {
  HRESULT hRet = E_FAIL;
  //Create DirectX.
  UINT Flags = 0;
  d3dsddesc = D3DXGetSwapChainDesc(hWnd, width, height);
  
  //DirectX11つくる
  hRet = D3D11CreateDeviceAndSwapChain(
    NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, Flags, NULL, 0,
    D3D11_SDK_VERSION, &d3dsddesc, &d3dswapchain, &d3ddevice, NULL, &d3dcontext);
  if(FAILED(hRet)) {
    char *errstr = "ERROR : D3D11CreateDeviceAndSwapChain";
    OutputDebugString(errstr);
    printf("%s\n", errstr);
    return hRet;
  }
  
  //バックバッファとる
  ID3D11Texture2D *pBackBuffer = NULL;
  hRet = d3dswapchain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
  if(FAILED(hRet)) {
    char *errstr = "ERROR : d3dswapchain->GetBuffer";
    OutputDebugString(errstr);
    printf("%s\n", errstr);
    return hRet;
  }
  
  //Viewに放り込む
  hRet = d3ddevice->CreateRenderTargetView( pBackBuffer, NULL, &d3drtv );
  RELEASE(pBackBuffer);
  if(FAILED(hRet)) {
    char *errstr = "ERROR : CreateRenderTargetView";
    OutputDebugString(errstr);
    printf("%s\n", errstr);
    return hRet;
  }
  d3dcontext->OMSetRenderTargets( 1, &d3drtv, NULL ); //サンプルだとエラーチェックしてない
  
  //ビューポート設定
  D3D11_VIEWPORT vp = {
    0.0f, 0.0f, (FLOAT)width, (FLOAT)height, 0.0f, 1.0f,
  };
  d3dcontext->RSSetViewports( 1, &vp );
  
  return hRet;
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
  if(!::RegisterClassEx(&wcex)) return 1;
  hWnd = ::CreateWindowEx(WS_EX_APPWINDOW | (Windowed ? WS_EX_WINDOWEDGE : 0),
    AppName, AppName, Windowed  ? WS_OVERLAPPEDWINDOW : WS_POPUP,
    0, 0, ScreenX, ScreenY, NULL, NULL, wcex.hInstance, NULL);
  if(!hWnd) {
    OutputDebugString("ERROR : Can't Create Window");
    return 0;
  }

  ShowWindow(hWnd, SW_SHOW);
  UpdateWindow(hWnd);
  Windowed ? (void)0 : ShowCursor(FALSE);
  
  //Create DirectX11 and start.
  if(SetupDevice(hWnd, ScreenX, ScreenY) == S_OK){
    BOOL bDone = FALSE;
    InitScene();
    while(!bDone) {
      while(::PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        if(msg.message == WM_QUIT) {
          bDone = TRUE;
          break;
        } else {
          ::TranslateMessage(&msg);
          ::DispatchMessage(&msg);
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

