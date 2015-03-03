//-----------//-----------//-----------//-----------//-----------//-----------
//
//
//  common.h
//
//
//-----------//-----------//-----------//-----------//-----------//-----------
#ifndef _COMMON_H_
#define _COMMON_H_

//-----------//-----------//-----------//-----------//-----------//-----------
// include
//-----------//-----------//-----------//-----------//-----------//-----------
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx9.h>
#include <d3dcommon.h>
#include <D3Dcompiler.h>
#include <xnamath.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "param.h"

//-----------//-----------//-----------//-----------//-----------//-----------
// lib
//-----------//-----------//-----------//-----------//-----------//-----------
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "dinput8.lib")

#pragma warning(disable:4244)
#pragma warning(disable:4305)

//-----------//-----------//-----------//-----------//-----------//-----------
// macrot
//-----------//-----------//-----------//-----------//-----------//-----------
#define RELEASE(x)            { if(x) (x)->Release(); x = NULL; }
#define S_RETURN(str, x)      { HRESULT hr; if( (hr = x) != S_OK) { printf("ERROR : %s : %d 0x%08X : %s\n", __FILE__, __LINE__, hr, str); return; } }
#define E_MSG(str, x)         { HRESULT hr; if( (hr = x) != S_OK) { printf("ERROR : %s : %d 0x%08X : %s\n", __FILE__, __LINE__, hr, str); } }
#define E_RETURN(str, x)      { HRESULT hr; if( (hr = x) != S_OK) { printf("ERROR : %s : %d 0x%08X : %s\n", __FILE__, __LINE__, hr, str); return hr; } }
#define E_NULL_RETURN(str, x) {             if(!(x))              { printf("ERROR : %s : %d NULL   : %s\n", __FILE__, __LINE__,     str); return E_FAIL; } }
#define SLEEP_60HZ            16//yam

//-----------//-----------//-----------//-----------//-----------//-----------
// mesh
//-----------//-----------//-----------//-----------//-----------//-----------
struct D3DX11MeshData {
  ID3D11Buffer *pVB;
  ID3D11Buffer *pIB;
  DWORD NumVertices;
  DWORD NumIndex;
  //ÇŸÇÒÇ∆ÇÕlayoutÇ‡àÍèèÇ…Ç∑Ç◊Ç´ÇæÇØÇ«ÇµÇ©Ç∆
};


//-----------//-----------//-----------//-----------//-----------//-----------
// func
//-----------//-----------//-----------//-----------//-----------//-----------
extern int          argc;
extern char       **argv;
extern LPSTR        AppName;
extern HWND         hWnd;
extern HRESULT      InitDevice(HWND hWnd, UINT width ,UINT height);
extern HRESULT      TermDevice();
extern void         InitScene();
extern void         TermScene();
extern void         DoScene();
extern HRESULT      D3DX11CreateMesh(int num, D3DX11MeshData *data);
extern void         D3DX11ReleaseMesh(D3DX11MeshData *data);


#endif //_COMMON_H_
