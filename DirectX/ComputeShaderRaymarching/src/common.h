//-----------//-----------//-----------//-----------//-----------//-----------
//
//
//  common.h
//
//
//-----------//-----------//-----------//-----------//-----------//-----------
#ifndef _COMMON_H_
#define _COMMON_H_

#pragma warning(disable:4244)
#pragma warning(disable:4305)
#pragma warning(disable:4005)
#pragma warning(disable:4996)

//-----------//-----------//-----------//-----------//-----------//-----------
// include
//-----------//-----------//-----------//-----------//-----------//-----------
#include <windows.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <D3Dcompiler.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <vector>
#include <string>
#include "param.h"

//-----------//-----------//-----------//-----------//-----------//-----------
// lib
//-----------//-----------//-----------//-----------//-----------//-----------
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winmm.lib")

//-----------//-----------//-----------//-----------//-----------//-----------
// macrot
//-----------//-----------//-----------//-----------//-----------//-----------
#define RELEASE(x)            { if(x) (x)->Release(); x = NULL; }
#define S_RETURN(str, x)      { HRESULT hr; if( (hr = x) != S_OK) { printf("ERROR : %s : %d 0x%08X : %s\n", __FILE__, __LINE__, hr, str); return; } }
#define E_MSG(str, x)         { HRESULT hr; if( (hr = x) != S_OK) { printf("ERROR : %s : %d 0x%08X : %s\n", __FILE__, __LINE__, hr, str); } }
#define E_RETURN(str, x)      { HRESULT hr; if( (hr = x) != S_OK) { printf("ERROR : %s : %d 0x%08X : %s\n", __FILE__, __LINE__, hr, str); return hr; } }
#define E_NULL_RETURN(str, x) {             if(!(x))              { printf("ERROR : %s : %d NULL   : %s\n", __FILE__, __LINE__,     str); return E_FAIL; } }
#define SLEEP_60HZ            16//yam

struct float4 {
	float x, y, z, w;
};

struct matrix {
	float4 m[4];
};


struct ShaderConst {
  float4   Const;
  matrix   mWorld;
  matrix   mView;
  matrix   mProj;
  FLOAT    Indicate;
};

//-----------//-----------//-----------//-----------//-----------//-----------
// func
//-----------//-----------//-----------//-----------//-----------//-----------
extern int                      argc;
extern char                   **argv;
extern LPSTR                    AppName;
extern HWND                     hWnd;
extern void                     show_fps();

extern ID3D11Device            *d3ddevice    ;
extern ID3D11DeviceContext     *d3dcontext   ;
extern IDXGISwapChain          *d3dswapchain ;
extern ID3D11Texture2D         *pBackTexture ;
extern ID3D11RenderTargetView  *pBackRTV     ;
extern ID3D11VertexShader      *pVShader     ;
extern ID3D11PixelShader       *pPShader     ;
extern ID3D11Buffer            *pConstant    ;
extern ID3D11InputLayout       *pVLayout     ;
extern ID3D11Buffer            *pVertexScreen;

HRESULT               D3DInitDevice(HWND hWnd, UINT width ,UINT height, LPCSTR fxfilename);
HRESULT               D3DTermDevice();
void                  D3DPresent(int vsync);
void                  D3DTermPixelShader();
HRESULT               D3DInitPixelShader(LPCSTR fxfilename);


HRESULT D3DCreateBuffer(
  ID3D11Buffer **ppBuffer,
  UINT BindFlags,
  UINT ByteWidth,
  D3D11_SUBRESOURCE_DATA *data = NULL,
  D3D11_USAGE Usage = D3D11_USAGE_DEFAULT,
  UINT AccessFlag = 0,
  UINT MiscFlags = 0,
  UINT StructureByteStride = 0);

HRESULT D3DCompileShaderFromFile( LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

HRESULT D3DCreateTexture(
  ID3D11Texture2D           **ppTex,
  ID3D11ShaderResourceView  **ppSRV,
  UINT         Width,
  UINT         Height,
  DXGI_FORMAT  Format    = DXGI_FORMAT_R32G32B32A32_FLOAT,
  D3D11_USAGE  Usage     = D3D11_USAGE_DEFAULT,
  UINT         BindFlags = D3D11_BIND_SHADER_RESOURCE,
  UINT         MipLevels = 1
);

HRESULT D3DCreateBufferUAV(ID3D11Buffer **pBuffer, ID3D11UnorderedAccessView **ppUAV, UINT ByteWidth, UINT Stride);
HRESULT D3DCreateSamplerState(ID3D11SamplerState **ppSamplerLinear, const D3D11_SAMPLER_DESC *pdesc = NULL);


//////////////////////////////////////////////////////////////////////////////////
// Util
//////////////////////////////////////////////////////////////////////////////////
class File {
	FILE *fp;
	std::vector<unsigned char> buf;
	int size;
public:
	File() { fp = NULL; size = 0; }
	~File() { Close(); }

	void Close() {
		if(fp) fclose(fp);
		fp = NULL;
		buf.resize(0);
	}

	int Open(const char *fname, const char *mode) {
		Close();
		fp = fopen(fname, mode);
		if(!fp) return -1;

		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		buf.resize(size);
		fread(&buf[0], 1, size, fp);

		fclose(fp);
		return buf.size();
	}
	void *Buf() { return (void *)&buf[0]; }
	int Size() { return size; }
};


//Entry
void    InitScene();
void    TermScene();
void    DoScene();

#endif //_COMMON_H_
