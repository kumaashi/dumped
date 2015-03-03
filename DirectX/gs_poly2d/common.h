#ifndef _COMMON_H_
#define _COMMON_H_


#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcommon.h>
#include <D3Dcompiler.h>
#include <xnamath.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "dinput8.lib")

#pragma warning(disable:4244)
#pragma warning(disable:4305)

#define RELEASE(x)         { if(x) (x)->Release(); x = NULL; }

#define S_RETURN(str, x) { HRESULT hr; if((hr = x) != S_OK) { printf("ERROR : %s : %d 0x%08X : %s\n", __FILE__, __LINE__, hr, str); return; } }
#define M_RETURN(str, x) { HRESULT hr; if((hr = x) != S_OK) { printf("ERROR : %s : %d 0x%08X\n", __FILE__, __LINE__, hr); return; } }

#define SLEEP_60HZ         16

#include "param.h"

extern int                     argc;
extern char                    **argv;
extern LPSTR                   AppName;
extern HWND                    hWnd;
extern DXGI_SWAP_CHAIN_DESC    d3dsddesc;
extern ID3D11Device            *d3ddevice   ;
extern ID3D11DeviceContext     *d3dcontext  ;
extern IDXGISwapChain          *d3dswapchain;
extern ID3D11RenderTargetView  *d3drtv      ;

void InitScene();
void TermScene();
void DoScene();

#endif

