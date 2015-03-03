//------------------------------------------------------------------------------
//
// DX11.CPP
//
//------------------------------------------------------------------------------	
//std
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

//++
#include <random>
#include <vector>
#include <iostream>

#include <windows.h>

#include <d3d11.h>
#include <d3dcommon.h>
//#include <d3d11_1.h>
//#include <d3dx11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>

//for ComPtr
#include <wrl/client.h>

//------------------------------------------------------------------------------
//
// pragma
//
//------------------------------------------------------------------------------
#pragma warning(disable:4244)
#pragma warning(disable:4305)
#pragma warning(disable:4005)
#pragma warning(disable:4996)
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "dinput8.lib")

//------------------------------------------------------------------------------
//
// define
//
//------------------------------------------------------------------------------
#define RELEASE(x)    { if(x) (x)->Release(); x = nullptr; }
#define amalloc(a, b) ((void *)(((unsigned long)(malloc(a + b))) & ~(b - 1)))

//------------------------------------------------------------------------------
//
// namespace
//
//------------------------------------------------------------------------------
using namespace DirectX;

//------------------------------------------------------------------------------
//fps : http://www.t-pot.com/program/13_fps/index.html
//------------------------------------------------------------------------------
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
// Random
//
//------------------------------------------------------------------------------
struct Random
{
	std::mt19937 rnd;
	unsigned long a, b, c;
	Random() : a(1), b(234567), c(890123) {}

	void Set(unsigned long  x,unsigned long  y, unsigned long z)
	{
		std::mt19937 t(x);
		rnd = t;
		
		a = x;
		b = y;
		c = z;
	}

	unsigned long Int()
	{
		return rnd();
		a += b;
		b += c;
		c += a;
		return (a >> 16);
	}
	
	float Float()
	{
		return -1 + (2 * float(Int()) / float(0xFFFFFFFF));
	}
};


//------------------------------------------------------------------------------
//
// Render
//
//------------------------------------------------------------------------------
struct Render
{
	struct Matrix
	{
		XMMATRIX World;
		XMMATRIX View;
		XMMATRIX Proj;
	};
	
	struct VData
	{
		XMFLOAT4 Pos;
		XMFLOAT4 Tex;
	};

	struct VIData
	{
		XMFLOAT4 Col;
		XMMATRIX M;
	};
	
	struct RectBatch
	{
		float px, py, pz;
		float sx, sy, sz;
		float rx, ry, rz;
		float r, g, b, a;
	};
	std::vector<RectBatch> vRectBatch;

	//Constant
	enum
	{
		VertexNum   = 6,
		InstanceMax = 32768,
	};
	
	//------------------------------------------------------------------------------
	// Var [POD obj]
	//------------------------------------------------------------------------------
	struct
	{
		int                     Width;
		int                     Height;
		HWND                    hWnd;
		ID3D11Device            *dev;
		IDXGIDevice1            *dxgi;
		IDXGIAdapter            *adapter;
		IDXGIFactory            *dxgiFactory;
		ID3D11DeviceContext     *ctx;
		IDXGISwapChain          *chain;
		ID3D11RenderTargetView  *RTV;
		ID3D11DepthStencilState *DSS;
		ID3D11DepthStencilView  *DSV;
		ID3D11Texture2D         *pDepthTexBuffer;

		ID3D11InputLayout       *IL;
		ID3D11VertexShader      *VS;
		ID3D11PixelShader       *PS;
		ID3D11GeometryShader    *GS;

		ID3D11BlendState        *BS;
		ID3D11RasterizerState   *RS;
		ID3D11Buffer            *VRect;
		ID3D11Buffer            *VRectIBuffer;
		Matrix                  matrix;
	} Var;

	//------------------------------------------------------------------------------
	// PrintDiag
	//------------------------------------------------------------------------------
	void Diag()
	{
		printf("ID3D11Device            *dev;                  %08X\n", Var.dev);
		printf("IDXGIDevice1            *dxgi;                 %08X\n", Var.dxgi);
		printf("IDXGIAdapter            *adapter;              %08X\n", Var.adapter);
		printf("IDXGIFactory            *dxgiFactory;          %08X\n", Var.dxgiFactory);
		printf("ID3D11DeviceContext     *ctx;                  %08X\n", Var.ctx);
		printf("IDXGISwapChain          *chain;                %08X\n", Var.chain);
		printf("ID3D11RenderTargetView  *RTV;                  %08X\n", Var.RTV);
		printf("ID3D11DepthStencilState *DSS;                  %08X\n", Var.DSS);
		printf("ID3D11DepthStencilView  *DSV;                  %08X\n", Var.DSV);
		printf("ID3D11Texture2D         *pDepthTexBuffer;      %08X\n", Var.pDepthTexBuffer);
		printf("ID3D11InputLayout       *IL;                   %08X\n", Var.IL);
		printf("ID3D11VertexShader      *VS;                   %08X\n", Var.VS);
		printf("ID3D11PixelShader       *PS;                   %08X\n", Var.PS);
		printf("ID3D11GeometryShader    *GS;                   %08X\n", Var.GS);
		printf("ID3D11BlendState        *BS;                   %08X\n", Var.BS);
		printf("ID3D11RasterizerState   *RS;                   %08X\n", Var.RS);
		printf("ID3D11Buffer            *VRect;                %08X\n", Var.VRect);
		printf("ID3D11Buffer            *VRectIBuffer;         %08X\n", Var.VRectIBuffer);
	}

	Render()
	{
		memset(&Var, 0, sizeof(Var));
	}

	~Render()
	{
		Term();
	}

	void Term()
	{
		DiscardShader();
		RELEASE(Var.VRectIBuffer);
		RELEASE(Var.VRect);
		RELEASE(Var.BS);
		RELEASE(Var.RS);
		RELEASE(Var.pDepthTexBuffer);
		RELEASE(Var.DSV);
		RELEASE(Var.DSS);
		RELEASE(Var.RTV);
		RELEASE(Var.chain);
		RELEASE(Var.ctx);
		RELEASE(Var.dxgiFactory);
		RELEASE(Var.dxgi);
		RELEASE(Var.adapter);
		RELEASE(Var.dev);
	}

	HRESULT CompileShaderFromFile( LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
	{
		HRESULT hr = S_OK;
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_PREFER_FLOW_CONTROL;
		ID3DBlob* pErrorBlob = nullptr;
		hr = D3DCompileFromFile( szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
		if(pErrorBlob)
		{
			printf("INFO : %s\n", (char *)pErrorBlob->GetBufferPointer());
			RELEASE(pErrorBlob);
		}
		printf("%s : %s %s::%08X\n", __FUNCTION__, szEntryPoint, szShaderModel, hr);
		return hr;
	}

	HRESULT CreateBufferUAV(ID3D11Buffer **pBuffer, ID3D11UnorderedAccessView **ppUAV, UINT ByteWidth, UINT Stride)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory( &uavDesc, sizeof(uavDesc) );
		D3D11_BUFFER_DESC bd =
		{
			ByteWidth, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, Stride
		};
		Var.dev->CreateBuffer( &bd, nullptr, pBuffer );
		uavDesc.Format              = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements	= ByteWidth / Stride;
		return Var.dev->CreateUnorderedAccessView( *pBuffer, &uavDesc, ppUAV );
	}

	HRESULT CreateTexture(ID3D11Texture2D **ppTex, ID3D11ShaderResourceView	**ppSRV, UINT	Width, UINT	Height, DXGI_FORMAT	Format, D3D11_USAGE	Usage, UINT	BindFlags, UINT	MipLevels)
	{
		static const D3D11_SHADER_RESOURCE_VIEW_DESC descTexture =
		{
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			D3D11_SRV_DIMENSION_TEXTURE2D, {0, 1}
		};

		D3D11_TEXTURE2D_DESC	 texdesc =
		{
			Width, Height, MipLevels, 1, Format,
			//{ 0, 0 },
			{ 1, 0 },
			Usage, BindFlags, 0, 0,
		};
		Var.dev->CreateTexture2D(&texdesc, 0, ppTex);
		return Var.dev->CreateShaderResourceView(*ppTex, &descTexture, ppSRV );
	}

	void CreateSamplerState(ID3D11SamplerState **ppSamplerLinear, const D3D11_SAMPLER_DESC *pdesc)
	{
		static const D3D11_SAMPLER_DESC sampDesc =
		{
			D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_WRAP,
			0, 0, D3D11_COMPARISON_NEVER,
			{0.0f, 0.0f, 0.0f, 0.0f},
			0,
			D3D11_FLOAT32_MAX
		};
		if(!pdesc) pdesc = &sampDesc;
		Var.dev->CreateSamplerState(pdesc, ppSamplerLinear);
	}
	
	//------------------------------------------------------------------------------
	// Reload
	//------------------------------------------------------------------------------
	void DiscardShader()
	{
		printf("PS=%08X\n", Var.PS);
		printf("GS=%08X\n", Var.GS);
		printf("VS=%08X\n", Var.VS);
		printf("IL=%08X\n", Var.IL);
		
		RELEASE(Var.PS);
		RELEASE(Var.GS);
		RELEASE(Var.VS);
		RELEASE(Var.IL);
	}
	
	//------------------------------------------------------------------------------
	// Reload
	//------------------------------------------------------------------------------
	void ReloadShader()
	{
		LPCWSTR filename = L"shader.hlsl";
		LPCSTR  vsentry  = "vs_main";
		LPCSTR  psentry  = "ps_main";
		LPCSTR  gsentry  = "gs_main";
		LPCSTR  pprofile = "ps_5_0";
		LPCSTR  vprofile = "vs_5_0";
		LPCSTR  gprofile = "gs_5_0";

		DiscardShader();

		printf("%s : ", __FUNCTION__);

		//------------------------------------------------------------------------------
		// CreateVertexShader and Layout
		//------------------------------------------------------------------------------
		{
			ID3DBlob* pBlob = nullptr;
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				{"POSITION",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16 * 0, D3D11_INPUT_PER_VERTEX_DATA,   0},
				{"TEXCOORD",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16 * 1, D3D11_INPUT_PER_VERTEX_DATA,   0},
				
				//For instancing
				{"COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16 * 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MATRIX",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16 * 1, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MATRIX",    1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16 * 2, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MATRIX",    2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16 * 3, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MATRIX",    3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16 * 4, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			};
			UINT numElements = ARRAYSIZE( layout );
			CompileShaderFromFile(filename, vsentry, vprofile, &pBlob);
			if(pBlob)
			{
				Var.dev->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &Var.VS );
				Var.dev->CreateInputLayout(layout, numElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &Var.IL );
			}
			RELEASE(pBlob);
		}
		
		//------------------------------------------------------------------------------
		//Geometry
		//------------------------------------------------------------------------------
		{
			ID3DBlob* pBlob = nullptr;
			CompileShaderFromFile(filename, gsentry, gprofile, &pBlob);
			if(pBlob)
			{
				Var.dev->CreateGeometryShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &Var.GS );
			}
			RELEASE(pBlob);
		}
		
		//------------------------------------------------------------------------------
		//Pixel
		//------------------------------------------------------------------------------
		{
			ID3DBlob* pBlob = nullptr;
			CompileShaderFromFile(filename, psentry, pprofile, &pBlob);
			if(pBlob)
			{
				Var.dev->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &Var.PS );
			}
			RELEASE(pBlob);
		}
		
		printf("Done . \n");
	}
	
	//------------------------------------------------------------------------------
	// Init
	//------------------------------------------------------------------------------
	void Init(HWND handle, int w, int h)
	{
		Var.hWnd = handle;
		Var.Width = w;
		Var.Height = h;
		
		//------------------------------------------------------------------------------
		//CreateDevice
		//------------------------------------------------------------------------------
		D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &Var.dev, nullptr, &Var.ctx);
		
		//------------------------------------------------------------------------------
		//Check MSAA
		//------------------------------------------------------------------------------
		DXGI_SAMPLE_DESC MSAA = {1,0};
		for(int i=0; i <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; i++)
		{
			UINT Quality;
			if SUCCEEDED(Var.dev->CheckMultisampleQualityLevels(DXGI_FORMAT_D24_UNORM_S8_UINT, i, &Quality))
			{
				if(0 < Quality)
				{
					MSAA.Count   = i;
					MSAA.Quality = Quality - 1;
					printf("MSAA %d, %d\n", MSAA.Count, MSAA.Quality);
				}
			}
		}
		//------------------------------------------------------------------------------
		//Create Swap Chain
		//------------------------------------------------------------------------------
		Var.dev->QueryInterface(__uuidof(IDXGIDevice1), (void**)&Var.dxgi);
		Var.dxgi->GetAdapter(&Var.adapter);
		Var.adapter->GetParent(__uuidof(IDXGIFactory), (void**)&Var.dxgiFactory);

		DXGI_SWAP_CHAIN_DESC scd;
		ZeroMemory(&scd, sizeof(scd) );
		scd.BufferCount        = 2;
		scd.BufferDesc.Width   = Var.Width;
		scd.BufferDesc.Height  = Var.Height;
		scd.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scd.BufferDesc.RefreshRate.Denominator = 1;
		scd.BufferDesc.RefreshRate.Numerator   = 0;
		scd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow       = Var.hWnd;
		scd.SampleDesc         = MSAA;
		scd.Windowed           = TRUE;
		scd.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;
		scd.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		Var.dxgiFactory->CreateSwapChain(Var.dev, &scd, &Var.chain);

		//------------------------------------------------------------------------------
		//get render target
		//------------------------------------------------------------------------------
		ID3D11Texture2D *pTexBuffer = nullptr;
		Var.chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pTexBuffer );
		Var.dev->CreateRenderTargetView(pTexBuffer, nullptr, &Var.RTV);
		RELEASE(pTexBuffer);

		//------------------------------------------------------------------------------
		// Create the texture for the depth buffer using the filled out description.
		//------------------------------------------------------------------------------
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
		depthBufferDesc.Width              = Var.Width;
		depthBufferDesc.Height             = Var.Height;
		depthBufferDesc.MipLevels          = 1;
		depthBufferDesc.ArraySize          = 1;
		depthBufferDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc         = MSAA;
		depthBufferDesc.Usage              = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags     = 0;
		depthBufferDesc.MiscFlags          = 0;
		Var.dev->CreateTexture2D(&depthBufferDesc, nullptr, &Var.pDepthTexBuffer);

		//------------------------------------------------------------------------------
		//Create DSV
		//------------------------------------------------------------------------------
		Var.dev->CreateDepthStencilView(Var.pDepthTexBuffer, nullptr, &Var.DSV);  //use mip 0

		//------------------------------------------------------------------------------
		// Set up the description of the stencil state.
		//------------------------------------------------------------------------------
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
		depthStencilDesc.DepthEnable                  = true;
		depthStencilDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc                    = D3D11_COMPARISON_LESS;
		depthStencilDesc.StencilEnable                = true;
		depthStencilDesc.StencilReadMask              = 0xFF;
		depthStencilDesc.StencilWriteMask             = 0xFF;
		depthStencilDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
		Var.dev->CreateDepthStencilState(&depthStencilDesc, &Var.DSS);

		//------------------------------------------------------------------------------
		//Create Vertex and Make rect data
		//------------------------------------------------------------------------------
		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory( &bd, sizeof(bd) );
			bd.ByteWidth       = sizeof( VData ) * VertexNum;
			bd.Usage           = D3D11_USAGE_DEFAULT;
			bd.BindFlags       = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags  = 0;

			D3D11_SUBRESOURCE_DATA InitData;
			ZeroMemory( &InitData, sizeof(InitData) );
			VData vertices[VertexNum] =
			{
				{XMFLOAT4(-1.0f, 0.0f, -1.0f, 1.0f), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f)},
				{XMFLOAT4( 1.0f, 0.0f,  1.0f, 1.0f), XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f)},
				{XMFLOAT4(-1.0f, 0.0f,  1.0f, 1.0f), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f)},
				{XMFLOAT4( 1.0f, 0.0f,  1.0f, 1.0f), XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f)},
				{XMFLOAT4(-1.0f, 0.0f, -1.0f, 1.0f), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f)},
				{XMFLOAT4( 1.0f, 0.0f, -1.0f, 1.0f), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f)},
			};			
			InitData.pSysMem = vertices;
			Var.dev->CreateBuffer( &bd, &InitData, &Var.VRect );
		}

		//------------------------------------------------------------------------------
		//Create Instance Buffer
		//------------------------------------------------------------------------------
		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory( &bd, sizeof(bd) );
			bd.ByteWidth       = sizeof(VIData) * InstanceMax,
			bd.Usage           = D3D11_USAGE_DYNAMIC;
			bd.BindFlags       = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags  = D3D11_CPU_ACCESS_WRITE;
			Var.dev->CreateBuffer( &bd, nullptr, &Var.VRectIBuffer );
		}

		//------------------------------------------------------------------------------
		// Blend
		//------------------------------------------------------------------------------
		D3D11_BLEND_DESC bsd;
		memset(&bsd, 0, sizeof(bsd));
		bsd.RenderTarget[0].BlendEnable    = true;
		bsd.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
		bsd.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
		bsd.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
		bsd.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
		bsd.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
		bsd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bsd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		Var.dev->CreateBlendState(&bsd, &Var.BS);

		//------------------------------------------------------------------------------
		// Cull [Create Cull None]
		//------------------------------------------------------------------------------
		D3D11_RASTERIZER_DESC rsd;
		ZeroMemory(&rsd, sizeof(rsd));
		//rsd.AntialiasedLineEnable = true; //too heavy
		rsd.AntialiasedLineEnable = false;

		rsd.MultisampleEnable     = true;
		rsd.CullMode              = D3D11_CULL_NONE ;
		rsd.DepthBias             = 0;
		rsd.DepthBiasClamp        = 0.0f;
		rsd.DepthClipEnable       = true;
		//rsd.FillMode              = D3D11_FILL_SOLID;
		rsd.FillMode              = D3D11_FILL_WIREFRAME;
		rsd.FrontCounterClockwise = false;
		rsd.ScissorEnable         = false;
		rsd.SlopeScaledDepthBias  = 0.0f;
		Var.dev->CreateRasterizerState(&rsd, &Var.RS);

		//------------------------------------------------------------------------------
		// Setup View port 
		//------------------------------------------------------------------------------
		static const	D3D11_VIEWPORT vp =
		{
			0.0f, 0.0f, (FLOAT)Var.Width, (FLOAT)Var.Height, 0.0f, 1.0f,
		};
		Var.ctx->RSSetViewports(1, &vp );

		//Reload Shader
		ReloadShader();
		
		//debug
		Diag();
	}

	//------------------------------------------------------------------------------
	// Clear
	//------------------------------------------------------------------------------	
	void Clear(float r, float g, float b, float a)
	{
		float col[4] = { r, g, b, a };
		Var.ctx->ClearRenderTargetView(Var.RTV, col);
		Var.ctx->ClearDepthStencilView(Var.DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	//------------------------------------------------------------------------------
	// Swap
	//------------------------------------------------------------------------------	
	void Swap(int sync)
	{
		Var.chain->Present(sync, 0);
	}

	//------------------------------------------------------------------------------
	// Proj
	//------------------------------------------------------------------------------	
	void Proj(float fov, float aspect, float nearz, float farz)
	{
		Var.matrix.Proj = XMMatrixPerspectiveFovLH(fov, aspect, nearz, farz);
	}
	
	//------------------------------------------------------------------------------
	// View
	//------------------------------------------------------------------------------	
	void View(
		float px,  float py,  float pz, 
		float ax,  float ay,  float az, 
		float ux,  float uy,  float uz)
	{
		Var.matrix.View = XMMatrixLookAtLH(
			XMLoadFloat3(&XMFLOAT3(px, py, pz)),
			XMLoadFloat3(&XMFLOAT3(ax, ay, az)),
			XMLoadFloat3(&XMFLOAT3(ux, uy, uz))
		);
	}

	//------------------------------------------------------------------------------
	// PushRect
	//------------------------------------------------------------------------------	
	void PushRect(
		float px, float yy, float xz,
		float sx, float sy, float sz,
		float rx, float ry, float rz,
		float r, float g, float b, float a)
	{
		RectBatch data =
		{
		 px,  yy,  xz,
		 sx,  sy,  sz,
		 rx,  ry,  rz,
		 r,  g,  b,  a
		};
		vRectBatch.push_back(data);
	}

	//------------------------------------------------------------------------------
	// Draw
	//------------------------------------------------------------------------------	
	void Draw()
	{
		int remain = vRectBatch.size();
		if(remain <= 0 || !Var.IL || !Var.RS || !Var.VS || !Var.PS || !Var.GS )
		{
			return;
		}
		int temp  = 0;
		int index = 0;

		//Setup IA -> 0:Vertex, 1:Tex,Color,WorldMatrix
		ID3D11Buffer *bptr[2] = { Var.VRect, Var.VRectIBuffer };
		UINT strides[2] = { sizeof(VData), sizeof(VIData) };
		UINT offsets[2] = { 0, 0 };
		Var.ctx->IASetVertexBuffers(0, 2, bptr, strides, offsets);
		Var.ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Var.ctx->IASetInputLayout(Var.IL);
		Var.ctx->IASetIndexBuffer( nullptr, DXGI_FORMAT_UNKNOWN, 0 );

		//Setup Shader
		Var.ctx->RSSetState(Var.RS);
		Var.ctx->VSSetShader(Var.VS, 0, NULL);
		Var.ctx->GSSetShader(Var.GS, 0, NULL);
		Var.ctx->PSSetShader(Var.PS, 0, NULL);
		
		//Setup OM
		float bf = 1.0;
		float BlendFactor[] = {bf, bf, bf, 0};
		Var.ctx->OMSetDepthStencilState(Var.DSS, 1);
		Var.ctx->OMSetRenderTargets(1, &Var.RTV, Var.DSV);
		Var.ctx->OMSetBlendState(Var.BS, BlendFactor, 0xFFFFFFFF);

		//Create View Proj M
		XMMATRIX vp  = XMMatrixMultiply(Var.matrix.View, Var.matrix.Proj);

		//Draw
		while(remain > 0)
		{
			//Setup Primitive Num
			temp = remain > InstanceMax ? InstanceMax : remain;
			
			//Make Instance Buffer
			D3D11_MAPPED_SUBRESOURCE m;
			if(Var.ctx->Map(Var.VRectIBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m) == S_OK)
			{
				VIData *buffer = (VIData *)m.pData;
				RectBatch *rect = &(vRectBatch[index]);
#pragma omp parallel for
				for(int i = 0; i < temp; i++)
				{
					XMMATRIX rotate = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&XMFLOAT3(rect[i].rx, rect[i].ry, rect[i].rz)));
					XMMATRIX trans  = XMMatrixTranslation(rect[i].px, rect[i].py, rect[i].pz);
					XMMATRIX scale  = XMMatrixScaling(rect[i].sx, rect[i].sy, rect[i].sz);
					XMMATRIX wvp    = XMMatrixMultiply(scale, rotate);
					buffer[i].Col   = XMFLOAT4(rect[i].r, rect[i].g, rect[i].b, rect[i].a);
					buffer[i].M     = XMMatrixTranspose( XMMatrixMultiply( XMMatrixMultiply( XMMatrixMultiply(scale, rotate), trans), vp));
				}
				Var.ctx->Unmap(Var.VRectIBuffer, 0);
			}

			//Draw
			Var.ctx->DrawInstanced(VertexNum, temp, 0, 0);

			//update
			remain -= temp;
			index  += temp;
		}
		vRectBatch.clear();
	}
};


//------------------------------------------------------------------------------	
//
// Windows
//
//------------------------------------------------------------------------------	
namespace Win
{
	HWND   hWnd     = nullptr;
	
	static LRESULT WINAPI WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

	int DoAppEvent()
	{
		MSG	msg;
		BOOL bDone = FALSE;
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				bDone = TRUE;
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		return bDone;
	}

	HWND Init(char *AppName, int Width, int Height, BOOL IsWindow  = TRUE)
	{
		RECT   rc;
		DWORD  Style    = IsWindow ? WS_OVERLAPPEDWINDOW : WS_POPUP;
		DWORD  StyleEx  = WS_EX_APPWINDOW | (IsWindow ? WS_EX_WINDOWEDGE : 0);
		
		static WNDCLASSEX wcex =
		{
			sizeof(WNDCLASSEX), CS_CLASSDC | CS_VREDRAW | CS_HREDRAW,
			WindowProc, 0L, 0L, nullptr, LoadIcon(nullptr, IDI_APPLICATION),
			LoadCursor(nullptr, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH),
			nullptr, AppName, nullptr
		};
		
		SetRect( &rc, 0, 0, Width, Height );
		AdjustWindowRectEx( &rc, Style, FALSE, StyleEx);
		rc.right	-= rc.left;
		rc.bottom -= rc.top;
		
		wcex.hInstance = GetModuleHandle(nullptr);
		if(!RegisterClassEx(&wcex)) return nullptr;
		hWnd = CreateWindowEx(StyleEx, AppName, AppName, Style, (GetSystemMetrics( SM_CXSCREEN ) - rc.right)	/ 2, (GetSystemMetrics( SM_CYSCREEN ) - rc.bottom) / 2, rc.right, rc.bottom, nullptr, nullptr, wcex.hInstance, nullptr);
		if(!hWnd) return nullptr;
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
		IsWindow ? (void)0 : ShowCursor(FALSE);
		
		return hWnd;
	}
} //win




#define WindowX  1280
#define WindowY  720
#define ScreenX  WindowX
#define ScreenY  WindowY
int main(int argc, char *argv[])
{
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);

	Render dx;
	dx.Init(Win::Init("DX11", ScreenX, ScreenY), ScreenX, ScreenY);
	Random rnd;
	while(!Win::DoAppEvent())
	{
		if(GetAsyncKeyState(VK_F5) & 0x8000)
		{
			dx.ReloadShader();
		}
		//dx.Clear(0.01, 0.02, 0.03, 1.0);
		dx.Clear(1, 1, 1, 1);
		//dx.DrawRect(0, 0, 1, 1);
		static float kkk = 0;
		kkk += 0.003;
		dx.View(
			//0, 2, -10,
				-cos(kkk) * 10, sin(kkk), sin(kkk) * 10,
			0, 0, 0,
			0, 1, 0);
		dx.Proj(
			60.0f * (3.14 / 180.0),
			float(WindowX)/float(WindowY), 0.1f, 256.0f);

		static float kk = 0;
		kk += 0.02;
		if(1)
		{
			static int count = 10000; //96700 x 6 
			if(GetAsyncKeyState(VK_RIGHT) & 0x8000)
			{
				printf("count = %d\n", count);
				count += 100;
			}

			rnd.Set(1, 2, 3);
			for(int i = 0;  i < count; i++)
			{
				float scale = 0.5;
				dx.PushRect(
					rnd.Float() * 20, 2 + rnd.Float() * 0.5, rnd.Float() * 20,
					0.5, 1.0, 0.5,
					//rnd.Float() + kk,rnd.Float() + kk,rnd.Float() + kk,
					0, rnd.Float() + kk,0,
					rnd.Float(),rnd.Float(),rnd.Float(), 1);
			}
			rnd.Set(1, 2, 3);
			for(int i = 0;  i < count; i++)
			{
				float scale = 0.5;
				dx.PushRect(
					rnd.Float() * 20,
					-2 + rnd.Float() * 0.5,
					rnd.Float() * 20,
					0.5, 01.0, 0.5,
					//rnd.Float() + kk,rnd.Float() + kk,rnd.Float() + kk,
					0, rnd.Float() + kk,0,
					rnd.Float(),rnd.Float(),rnd.Float(),  1);
			}
			
		}
		
		if(1)
		{
			dx.PushRect(
				0, 0, 0,
				1, 1, 1,
				0, kk, 0,
				0.5, 0.2, 0.3, 0.5);
			dx.PushRect(
				0, 0, 0,
				1, 1, 1,
				kk + 0.5, 0, 0,
				0.1, 0.2, 0.3, 0.5);
		}
		
		dx.Draw();
		dx.Swap( (GetAsyncKeyState(VK_F3) & 0x8000) ? 0 : 1);
		show_fps();
	}
	dx.Term();
	return 0;
}




