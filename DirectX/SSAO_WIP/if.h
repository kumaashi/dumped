#ifndef _IF_H_
#define _IF_H_
#include <atlbase.h>
#include <wrl/client.h>
#include <random>
#include <vector>
#include <iostream>
#include <windows.h>
#include <d3d11.h>
#include <d3dcommon.h>

#define  DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <windows.h>
#include <process.h>
#include <vector>
#include <map>

#pragma  warning(disable:4244)
#pragma  warning(disable:4305)
#pragma  warning(disable:4005)
#pragma  warning(disable:4996)

#pragma  comment(lib, "winmm.lib")
#pragma  comment(lib, "user32.lib")
#pragma  comment(lib, "gdi32.lib")
#pragma  comment(lib, "dxguid.lib")
#pragma  comment(lib, "d3d11.lib")
#pragma  comment(lib, "D3dcompiler.lib")
#pragma  comment(lib, "dinput8.lib")

#define  RELEASE(x)    { if(x) (x)->Release(); x = NULL; }
#define  amalloc(a, b) ((void *)(((unsigned long)(malloc(a + b))) & ~(b - 1)))
#define  KEYDOWN(name, key) (name[key] & 0x80)

using namespace Microsoft::WRL;
using namespace DirectX;


//------------------------------------------------------------------------------
// System
//------------------------------------------------------------------------------
class System {
public:
	HWND   hWnd;
	DWORD  Width;
	DWORD  Height;
	HWND GetHandle() { return hWnd; }
	static LRESULT WINAPI WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void Init(char *AppName, int w, int h, BOOL IsWindow = TRUE);
	bool IsDone();
	void Sleep() { ::Sleep(0); }
};

//------------------------------------------------------------------------------
// Input
//------------------------------------------------------------------------------
class Input {
	struct {
		CComPtr<IDirectInput8>        di;
		CComPtr<IDirectInputDevice8>  dikbd;
		CComPtr<IDirectInputDevice8>  dipad;
		LONG                          deadzone;
		BOOL                          diag;
	} state;

public:
	enum {
		Left = DIK_LEFT,
		Up = DIK_UP,
		Down = DIK_DOWN,
		Right = DIK_RIGHT,
		B0 = DIK_Z,
		B1 = DIK_X,
		B2 = DIK_C,
		B3 = DIK_V,
	};

	Input() {
		memset(&state, 0, sizeof(state));
		state.deadzone = 20000;
	}


	void SetDeadzone(LONG value) {
		state.deadzone = value;
	}

	void Term(){}

	static BOOL CALLBACK DIEnumGamePadProc(LPDIDEVICEINSTANCE pDIDInst, LPVOID pRef);

	void Init(HWND hWnd) {
		HRESULT hRet = S_OK;
		DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&state.di, NULL);
		if (!state.di) {
			printf("%s Failed DirectInput8Create\n", __FUNCTION__);
			return;
		}

		//keyboard
		hRet |= state.di->CreateDevice(GUID_SysKeyboard, &state.dikbd, NULL);
		if (!state.dikbd) {
			printf("%s Failed CreateDevice GUID_SysKeyboard\n", __FUNCTION__);
			return;
		}
		hRet |= state.dikbd->SetDataFormat(&c_dfDIKeyboard);
		//hRet |= dikbd->SetCooperativeLevel((HWND)System::GetHandle(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		hRet |= state.dikbd->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
		hRet |= state.dikbd->Acquire();
		if (hRet)
			printf("%s Failed intiialize \n", __FUNCTION__);
		else
			printf("%s S_OK intiialize \n", __FUNCTION__);

		//sub joystick
		state.di->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACK)DIEnumGamePadProc, this, DIEDFL_ATTACHEDONLY);
		if (state.dipad)
		{
			HRESULT hRet = S_OK;
			hRet |= state.dipad->SetDataFormat(&c_dfDIJoystick2);
			//hRet |= dipad->SetCooperativeLevel((HWND)System::GetHandle(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
			hRet |= state.dipad->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
			hRet |= state.dipad->Acquire();
			printf("%s : dipad=%08X hRet = %08X\n", __FUNCTION__, state.dipad, hRet);
		}
	}

	int Get(int id, int isanalog = 1) {
		int ret = 0;
		char     buffer[256];
		HRESULT  hr;
		hr = state.dikbd->GetDeviceState(sizeof(buffer), (LPVOID)&buffer);
		if FAILED(hr) {
			state.dikbd->Acquire();
			printf("%s Failed GetDeviceState \n", __FUNCTION__);
			return 0;
		}

		// Turn the spaceship right or left
		if (KEYDOWN(buffer, id)) {
			ret = 1;
		}

		if (state.dipad) {
			DIJOYSTATE2 js = { 0 };
			if (state.dipad->Poll() == DIERR_INPUTLOST) {
				printf("%s Failed Poll DIERR_INPUTLOST\n", __FUNCTION__);
				state.dipad->Acquire();
			}

			if (FAILED(state.dipad->GetDeviceState(sizeof(DIJOYSTATE2), (LPVOID)&js))) {
				printf("%s Failed GetDeviceState dipad\n", __FUNCTION__);
				state.dipad->Acquire();
			}

			if (state.diag) {
				for (int i = 0; i < sizeof(js.rgbButtons); i++) {
					if (KEYDOWN(js.rgbButtons, i)) {
						printf("%d:", i);
					}
				}
			}
			if (id == DIK_LEFT)  ret |= (js.lX) <= (0x0000 + state.deadzone) ? 1 : 0;
			if (id == DIK_RIGHT) ret |= (js.lX) >= (0x7FFF + state.deadzone) ? 1 : 0;
			if (id == DIK_UP)    ret |= (js.lY) <= (0x0000 + state.deadzone) ? 1 : 0;
			if (id == DIK_DOWN)  ret |= (js.lY) >= (0x7FFF + state.deadzone) ? 1 : 0;
			if (id == DIK_Z)     ret |= (KEYDOWN(js.rgbButtons, 0)) ? 1 : 0;
			if (id == DIK_X)     ret |= (KEYDOWN(js.rgbButtons, 1)) ? 1 : 0;
			if (id == DIK_C)     ret |= (KEYDOWN(js.rgbButtons, 2)) ? 1 : 0;
			if (id == DIK_V)     ret |= (KEYDOWN(js.rgbButtons, 0)) ? 1 : 0;
		}
		return ret;
	}
};



//------------------------------------------------------------------------------
// Render
//------------------------------------------------------------------------------
class Render {
	struct {
		CComPtr<ID3D11VertexShader>  VS;
		CComPtr<ID3D11InputLayout>   IL;
		CComPtr<ID3D11PixelShader>   PS;
		CComPtr<ID3D11Buffer>        Buffer;
		UINT  strides;
		UINT  offsets;
		UINT  numvertices;
	} BackSwap;
	void InitBackSwap();
	void DrawBackSwap();
public:
	void LoadPreset();
	struct VData {
		XMFLOAT4 Pos;
		XMFLOAT4 Tex;
	};

	struct Matrix {
		XMMATRIX World;
		XMMATRIX View;
		XMMATRIX Proj;
	};

	struct VIData {
		XMFLOAT4 Col;
		XMMATRIX M;
	};

	struct RectBatch {
		float px, py, pz, pw;
		float sx, sy, sz, sw;
		float rx, ry, rz, rw;
		float r, g, b, a;
	};

	std::vector<RectBatch> vRectBatch;

	enum {
		VertexNum   = 6,
		InstanceMax = 8192,
		MeshMax     = 256,
	};


	struct Mesh {
		CComPtr<ID3D11Buffer>    Buffer;
		DWORD                    VertexNum;
		DWORD                    StrideSize;
	};


	//------------------------------------------------------------------------------
	// Var [POD obj]
	//------------------------------------------------------------------------------
	struct {
		int                                Width;
		int                                Height;
		HWND                               hWnd;

		CComPtr<ID3D11Device>              dev;
		CComPtr<ID3D11DeviceContext>       ctx;
		CComPtr<IDXGISwapChain>            chain;
		CComPtr<ID3D11RenderTargetView>    RTV;
		CComPtr<ID3D11DepthStencilView>    DSV;
		CComPtr<ID3D11Texture2D>           DTex;
		CComPtr<ID3D11DepthStencilState>   DSS;
		
		CComPtr<ID3D11InputLayout>         IL;
		CComPtr<ID3D11VertexShader>        VS;
		
		CComPtr<ID3D11PixelShader>         PS;
		CComPtr<ID3D11GeometryShader>      GS;
		CComPtr<ID3D11BlendState>          BS;
		CComPtr<ID3D11SamplerState>        SState;
		CComPtr<ID3D11Buffer>              VInstBuf;
		CComPtr<ID3D11RasterizerState>     RS[2];

		Matrix                             matrix;
		Mesh                               mesh[MeshMax];

		CComPtr<ID3D11Texture2D>           BackTex;
		CComPtr<ID3D11RenderTargetView>    BackRTV;
		CComPtr<ID3D11ShaderResourceView>  BackSRV;
		DXGI_SAMPLE_DESC                   MSAA;
		
		void Diag()
		{
			printf("======================================================\n");
			printf("dev          = 0x%08X\n", dev);
			printf("ctx          = 0x%08X\n", ctx);
			printf("chain        = 0x%08X\n", chain);
			printf("RTV          = 0x%08X\n", RTV);
			printf("DTex         = 0x%08X\n", DTex);
			printf("DSS          = 0x%08X\n", DSS);
			printf("DSV          = 0x%08X\n", DSV);
			printf("IL           = 0x%08X\n", IL);
			printf("VS           = 0x%08X\n", VS);
			printf("PS           = 0x%08X\n", PS);
			printf("GS           = 0x%08X\n", GS);
			printf("BS           = 0x%08X\n", BS);
			printf("SState       = 0x%08X\n", SState);
			printf("VInstBuf     = 0x%08X\n", VInstBuf);
			printf("BackTex      = 0x%08X\n", BackTex);
			printf("BackRTV      = 0x%08X\n", BackRTV);
			printf("BackSRV      = 0x%08X\n", BackSRV);
			printf("MSAA.Count   = %d\n",     MSAA.Count  );
			printf("MSAA.Quality = %d\n",     MSAA.Quality);
			printf("======================================================\n\n");
		}
	} Var;
	
	void Term() {}

	int Load(int index, void *data, int num, int stridesize);
	void ResetShader();
	void Init(HWND handle, int w, int h);
	void Clear(FLOAT ColorList[4]);
	void Swap(int sync);
	void Proj(float fov, float aspect, float nearz, float farz);
	void View(
	    float px, float py, float pz,
	    float ax, float ay, float az,
	    float ux, float uy, float uz);
	void PushRect(
	    float px, float yy, float xz,
	    float sx, float sy, float sz,
	    float rx, float ry, float rz,
	    float r, float g, float b, float a);
	void Draw(int id = 0);
	void Object(
	    float px, float py, float pz,
	    float rx, float ry, float rz,
	    float sx, float sy, float sz,
	    float r, float g, float b, float a);
	void Flip();
};


//------------------------------------------------------------------------------
// Audio
//------------------------------------------------------------------------------
class Audio
{
public:
	void Thread(void *args)
	{

	}

	void Init()
	{
		//_beginthread(Thread, NULL, NULL);
		//Sleep(1000);
	}

	void Load(int id, float *data, int num)
	{

	}

	void Play(int id)
	{
	}

};


//------------------------------------------------------------------------------
//
// Util
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//fps : http://www.t-pot.com/program/13_fps/index.html
//------------------------------------------------------------------------------
namespace Util
{

	struct Actor
	{
	};

	struct ShowFps
	{
		unsigned long last;
		unsigned long frames;
		char  buf[256];

		void Init()
		{
			last = timeGetTime();
			frames = 0;
			memset(buf, 0, sizeof(buf));
		}

		char  *Update()
		{
			frames++;
			unsigned long current = timeGetTime();
			if(1000 <= current - last)
			{
				float dt  = (float)(current - last) / 1000.0f;
				float fps = (float)frames / dt;
				last      = current;
				frames    = 0;
				sprintf(buf, "%.02f fps            ", fps);
				printf("%s\r", buf);
			}
			return buf;
		}
	};


	//------------------------------------------------------------------------------
	// Random
	//------------------------------------------------------------------------------
	struct Random
	{
		//std::mt19937 rnd;
		int a, b, c;
		Random() : a(1), b(234567), c(890123) {}

		void Set(int x = 1, int y = 456787, int z = 890123)
		{
			//std::mt19937 t(x);
			//rnd = t;
			a = x;
			b = y;
			c = z;
		}

		int Int(int index = 0)
		{
			//return rnd();
			a += b + index;
			b += c;
			c += a;
			return (a >> 16);
		}

		float Float(int index = 0)
		{
			return -1 + (2 * float(Int()) / float(0x7FFF));
			//return float(Int(index)) / float(0x7FFF);
		}
	};


}




#endif //_IF_H_
