//--------//--------//--------//--------//--------//--------//--------//--------//--------
//
//
// Direct2D and tools. 2013
//
//
//--------//--------//--------//--------//--------//--------//--------//--------//--------
#ifndef _COMMON_H_
#define _COMMON_H_

//--------//--------//--------//--------//--------//--------//--------//--------//--------
// tekitou
//--------//--------//--------//--------//--------//--------//--------//--------//--------
#pragma warning(disable:4244)
#pragma warning(disable:4305)
#pragma warning(disable:4005)
#pragma warning(disable:4996)

//--------//--------//--------//--------//--------//--------//--------//--------//--------
// include
//--------//--------//--------//--------//--------//--------//--------//--------//--------
#include <windows.h>
#include <dwrite.h>
#include <d2d1_1.h>
#include <vector>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

//--------//--------//--------//--------//--------//--------//--------//--------//--------
// lib
//--------//--------//--------//--------//--------//--------//--------//--------//--------
#pragma comment(lib,     "winmm.lib")
#pragma comment(lib,     "user32.lib")
#pragma comment(lib,     "gdi32.lib")
#pragma comment(lib,     "d2d1.lib")
#pragma comment(lib,     "dwrite.lib")
#pragma comment(lib,     "dinput8.lib")

//--------//--------//--------//--------//--------//--------//--------//--------//--------
// define
//--------//--------//--------//--------//--------//--------//--------//--------//--------
#define E_RETURN(str, x) { HRESULT hRet = (x); if(hRet != S_OK) { printf("E_RETURN(%08X) : %s\n", hRet, str); return hRet; } }
#define RELEASE(x)       { if(x) (x)->Release(); x = NULL; }
#define ASIZE(x)         ( sizeof(x) / sizeof(x[0]) )

//--------//--------//--------//--------//--------//--------//--------//--------//--------
//
//
// NAMESPACE common START
//
//
//--------//--------//--------//--------//--------//--------//--------//--------//--------
namespace common {

  //--------//--------//--------//--------//--------//--------//--------//--------//--------
  // vec
  //--------//--------//--------//--------//--------//--------//--------//--------//--------
  typedef float real;
  struct vec {
    real x, y, z;
    vec(real a = 0, real b = 0, real c = 0) : x(a), y(b), z(c) {}
    inline real dot(vec &a) { return x * a.x + y * a.y + z * a.z; }
    inline real len()       { return sqrt(dot(*this)); }
    inline vec  nor()       { real k = 1.0 / len(); return vec(x * k, y * k, z * k); }
    inline void disp()      { printf("%f %f %f : len = %f, \n", x, y, z, len()); }
    #define _vop_(op)         inline vec operator op (const vec &a) { return vec(x op a.x, y op a.y, z op a.z); }
    _vop_(+) _vop_(-) _vop_(*) _vop_(/) _vop_(+=) _vop_(-=) _vop_(*=) _vop_(/=) _vop_(<) _vop_(>) 
  };

  //--------//--------//--------//--------//--------//--------//--------//--------//--------
  // random
  //--------//--------//--------//--------//--------//--------//--------//--------//--------
  struct random {
    long a, b, c;
    random() { a = 1, b = 234567, c = 890123; }
    long  get()  { a += b; b += c; c += a; return (a >> 16); }
    float fget() { return float(get()) / float(0x7FFF); }
  };

  //--------//--------//--------//--------//--------//--------//--------//--------//--------
  //fps : http://www.t-pot.com/program/13_fps/index.html
  //--------//--------//--------//--------//--------//--------//--------//--------//--------
  inline void show_fps() {
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

  //--------//--------//--------//--------//--------//--------//--------//--------//--------
  //
  // DIRECT2D
  //
  //--------//--------//--------//--------//--------//--------//--------//--------//--------
  struct Direct2D {
    enum {
      BRUSH_MAX   = 256,
      BITMAP_MAX  = 256,
      TEXTFMT_MAX = 64,
    };
    
    ID2D1Factory           *pD2DFactory;
    ID2D1HwndRenderTarget  *pRT;
    ID2D1Bitmap            *pBitmap[BITMAP_MAX];
    ID2D1SolidColorBrush   *pBrush[BRUSH_MAX];
    ID2D1SolidColorBrush   *GetColor(DWORD index)      { return pBrush[index % BRUSH_MAX];  }
    
    IDWriteFactory         *pDWriteFactory;
    IDWriteTextFormat      *pTextFormat[TEXTFMT_MAX];
    IDWriteTextFormat      *GetTextFmt(DWORD index)    { return pTextFormat[index % TEXTFMT_MAX]; }
    
    HRESULT Init(HWND hWnd, DWORD x, DWORD y) {
      ZeroMemory(this, sizeof(Direct2D));
      E_RETURN("D2D1CreateFactory",      D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory));
      E_RETURN("CreateHwndRenderTarget", pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(x, y)), &pRT));
      float color  = 0, dcolor = 1.0 / float(BRUSH_MAX);
      for(auto i = 0 ; i < BRUSH_MAX; i++, color += dcolor) E_RETURN("CreateColor",          CreateColor(i, color, color, color, 1));
      E_RETURN("DWriteCreateFactory",    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(pDWriteFactory), (IUnknown **)&pDWriteFactory));
      return S_OK;
    }

    void Term() {
      for(auto i = 0 ; i < ASIZE(pTextFormat); i++) RELEASE(pTextFormat[i]);
      for(auto i = 0 ; i < ASIZE(pBrush);      i++) RELEASE(pBrush[i]);
      for(auto i = 0 ; i < ASIZE(pBitmap);     i++) RELEASE(pBitmap[i]);
      RELEASE(pRT);
      RELEASE(pD2DFactory);
    }

    HRESULT CreateColor(DWORD index, float r, float g, float b, float a = 1.0) {
      RELEASE(pBrush[index]);
      return pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(r, g, b, a)), &pBrush[index]);
    }
    
    HRESULT CreateTextFmt(DWORD index, WCHAR *name, UINT pitch) {
      RELEASE(pTextFormat[index]);
      return pDWriteFactory->CreateTextFormat(
          name, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, pitch, L"", &pTextFormat[index]);
    }

    //hate.
    HRESULT CreateBitmap(DWORD index, void *buf, DWORD w, DWORD h, DWORD stride) {
      D2D1_PIXEL_FORMAT format = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
      RELEASE(pBitmap[index]);
      return pRT->CreateBitmap(D2D1::SizeU(w, h), buf, stride, D2D1::BitmapProperties(format), &pBitmap[index]);
    }

    //--------//--------//--------//--------//--------//--------//--------//--------//--------
    // render and draw
    //--------//--------//--------//--------//--------//--------//--------//--------//--------
    void Render(void (CallBack)(Direct2D *)) {
      pRT->BeginDraw();
      pRT->SetTransform(D2D1::Matrix3x2F::Identity());
      CallBack(this);
      pRT->EndDraw();
    }
    
    void Clear(float r = 0, float g = 0, float b = 0, float a = 0) { pRT->Clear(D2D1::ColorF(r, g, b, a)); }
    void SetTransform(float x, float y, float angle) {
      pRT->SetTransform(
        D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(0, 0)) *
        D2D1::Matrix3x2F::Translation(D2D1::SizeF(x, y)));
    }
    
    void DrawText(DWORD fmt, WCHAR *text, INT len, float x, float y, DWORD color, float angle = 0) {
      if(!text) return;
      if(len < 0) len = wcslen(text);
      D2D1_SIZE_F rtSize = pRT->GetSize();
      const D2D1_RECT_F trect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);
      SetTransform(x, y, angle);
      pRT->DrawText(text, len, GetTextFmt(fmt), trect, GetColor(color));
    }

    void DrawEllipse(float x, float y, float w, float h, float angle = 0, DWORD color = 128, BOOL fill = FALSE, float k = 1.0) {
      D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(0, 0), w, h );
      pRT->SetTransform(D2D1::Matrix3x2F::Identity());
      SetTransform(x, y, angle);
      if(fill)  pRT->FillEllipse(ellipse, GetColor(color));
      else      pRT->DrawEllipse(ellipse, GetColor(color), k);
    }
    
    void DrawRect(float x, float y, float w, float h, float angle = 0, DWORD color = 128, BOOL fill = FALSE, float k = 1.0) {
      const D2D1_RECT_F rect = D2D1::RectF(-w, -h, w, h);
      pRT->SetTransform(D2D1::Matrix3x2F::Identity());
      SetTransform(x, y, angle);
      if(fill) pRT->FillRectangle(rect, GetColor(color));
      else     pRT->DrawRectangle(rect, GetColor(color), k);
    }
    
    void DrawGrid(int sidew, int ox = 0, int oy = 0, DWORD color = 0, float lw = 0.5) {
      D2D1_SIZE_F rtSize = pRT->GetSize();
      int width  = int(rtSize.width);
      int height = int(rtSize.height);
      for (auto x = ox % sidew; x < width;  x += sidew)
        pRT->DrawLine(D2D1::Point2F(float(x), 0.0f), D2D1::Point2F(float(x), rtSize.height), GetColor(color), lw );
      for (auto y = oy % sidew; y < height; y += sidew)
        pRT->DrawLine(D2D1::Point2F(0.0f, float(y)), D2D1::Point2F(rtSize.width, float(y)),  GetColor(color), lw );
    }
  };


  //---------//---------//---------//---------//---------//---------//---------//---------
  // Make Window
  //---------//---------//---------//---------//---------//---------//---------//---------
  LRESULT WINAPI SimpleWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
  inline HWND InitWindow(LPCSTR AppName, DWORD w, DWORD h, LRESULT WINAPI WindowProc(HWND , UINT , WPARAM , LPARAM ) = SimpleWindowProc) {
    RECT  rc;
    HWND  hWnd     = NULL;
    BOOL  Windowed = TRUE;
    DWORD Style   =  Windowed ? WS_OVERLAPPEDWINDOW : WS_POPUP;
    DWORD StyleEx = WS_EX_APPWINDOW | (Windowed ? WS_EX_WINDOWEDGE : 0);
    SetRect( &rc, 0, 0, w, h );
    AdjustWindowRectEx( &rc, Style, FALSE, StyleEx);
    rc.right  -= rc.left;
    rc.bottom -= rc.top;
    static WNDCLASSEX wcex = {
      sizeof(WNDCLASSEX), CS_CLASSDC | CS_VREDRAW | CS_HREDRAW,
      WindowProc, 0L, 0L, GetModuleHandle(NULL), LoadIcon(NULL, IDI_APPLICATION),
      LoadCursor(NULL, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH),
      NULL, AppName, NULL
    };
    if(!RegisterClassEx(&wcex)) return NULL;
    hWnd = CreateWindowEx(
      StyleEx, AppName, AppName, Style,
      (GetSystemMetrics( SM_CXSCREEN ) - rc.right) / 2, (GetSystemMetrics( SM_CYSCREEN ) - rc.bottom) / 2,
      rc.right, rc.bottom, NULL, NULL, wcex.hInstance, NULL);
    if(!hWnd) {
      printf("ERROR : Can't Create Window");
      return NULL;
    }
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    Windowed ? (void)0 : ShowCursor(FALSE);
    return hWnd;
  }

  //---------//---------//---------//---------//---------//---------//---------//---------
  // Process Message 
  //---------//---------//---------//---------//---------//---------//---------//---------
  inline BOOL ProcMsg() {
    MSG msg;
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
      if(msg.message == WM_QUIT) return FALSE;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    return TRUE;
  }

  inline LRESULT WINAPI SimpleWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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

} //namespace common
//--------//--------//--------//--------//--------//--------//--------//--------//--------
//
//
// NAMESPACE common END
//
//
//--------//--------//--------//--------//--------//--------//--------//--------//--------

#endif //_COMMON_H_
