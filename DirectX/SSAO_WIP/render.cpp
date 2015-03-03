#include "if.h"

HRESULT CompileShaderFromFile(LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_PREFER_FLOW_CONTROL;
	CComPtr<ID3DBlob> pErrorBlob = NULL;
	hr = D3DCompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (pErrorBlob)
	{
		printf("ERROR : %s\n", (char *)pErrorBlob->GetBufferPointer());
	}
	//printf("%s : %s %s::%08X\n", __FUNCTION__, szEntryPoint, szShaderModel, hr);
	return hr;
}

int Render::Load(int index, void *data, int num, int stridesize)
{
	Mesh *w = &Var.mesh[index];
	D3D11_BUFFER_DESC bd = { 0 };
	D3D11_SUBRESOURCE_DATA subdata = { 0 };
	bd.ByteWidth      = stridesize * num;
	bd.Usage          = D3D11_USAGE_DEFAULT;
	bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	subdata.pSysMem   = data;
	w->VertexNum      = num;
	w->StrideSize     = stridesize;
	return Var.dev->CreateBuffer(&bd, &subdata, &w->Buffer);
}


//------------------------------------------------------------------------------
// Reload
//------------------------------------------------------------------------------
void Render::ResetShader()
{
	LPCWSTR filename        = L"shader.hlsl";
	LPCSTR  vsentry         = "vs_main";
	LPCSTR  psentry         = "ps_main";
	LPCSTR  gsentry         = "gs_main";
	LPCSTR  pprofile        = "ps_5_0";
	LPCSTR  vprofile        = "vs_5_0";
	LPCSTR  gprofile        = "gs_5_0";
	CComPtr<ID3DBlob> pBlob = NULL;

	printf("[%s]\n", __FUNCTION__);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16 * 0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16 * 1, D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16 * 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "MATRIX",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16 * 1, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "MATRIX",   1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16 * 2, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "MATRIX",   2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16 * 3, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "MATRIX",   3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16 * 4, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	CompileShaderFromFile(filename, vsentry, vprofile, &pBlob);
	if (pBlob) {
		Var.dev->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &Var.VS);
		Var.dev->CreateInputLayout(layout, ARRAYSIZE(layout), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &Var.IL);
	} else {
		printf("ERROR %s : Not found file %s", __FUNCTION__, filename);
	}
	CompileShaderFromFile(filename, gsentry, gprofile, &pBlob);
	if (pBlob) {
		Var.dev->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &Var.GS);
	} else {
		printf("ERROR %s : CreateGeometryShader", __FUNCTION__);
	}
	CompileShaderFromFile(filename, psentry, pprofile, &pBlob);
	if (pBlob) {
		Var.dev->CreatePixelShader(pBlob->GetBufferPointer(),    pBlob->GetBufferSize(), NULL, &Var.PS);
	} else {
		printf("ERROR %s : CreatePixelShader", __FUNCTION__);
	}

	Var.Diag();
}

//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
void Render::Init(HWND handle, int w, int h) {
	Var.hWnd   = handle;
	Var.Width  = w;
	Var.Height = h;
	D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &Var.dev, NULL, &Var.ctx);
	DXGI_SAMPLE_DESC MSAA        = { 1, 0 };
	DXGI_SAMPLE_DESC MSAADefault = MSAA;
	
	if(0) {
		for (int i = 0; i <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; i++) {
			UINT Quality;
			if SUCCEEDED(Var.dev->CheckMultisampleQualityLevels(DXGI_FORMAT_D24_UNORM_S8_UINT, i, &Quality)) {
				if (0 < Quality) {
					MSAA.Count = i;
					MSAA.Quality = Quality - 1;
					printf("MSAA %d, %d\n", MSAA.Count, MSAA.Quality);
				}
			}
		}
	}
	Var.MSAA = MSAA;

	CComPtr<IDXGIDevice1>    dxgi          = NULL;
	CComPtr<IDXGIAdapter>    adapter       = NULL;
	CComPtr<IDXGIFactory>    dxgiFactory   = NULL;
	Var.dev->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgi);
	dxgi->GetAdapter(&adapter);
	adapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferCount                        = 1;
	scd.BufferDesc.Width                   = Var.Width;
	scd.BufferDesc.Height                  = Var.Height;
	scd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.RefreshRate.Numerator   = 0;
	scd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow                       = Var.hWnd;
	scd.SampleDesc                         = Var.MSAA;
	scd.Windowed                           = TRUE;
	scd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	dxgiFactory->CreateSwapChain(Var.dev , &scd, &Var.chain);

	printf("dxgi       :%08X\n", dxgi);
	printf("adapter    :%08X\n", adapter);
	printf("dxgiFactory:%08X\n", dxgiFactory);

	//Get Back Buffer with RTV.
	CComPtr<ID3D11Texture2D> tempTex = NULL;
	Var.chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&tempTex);
	Var.dev->CreateRenderTargetView(tempTex, NULL, &Var.RTV);

	// Create Depth Stencli Buffer
	D3D11_TEXTURE2D_DESC    DTexDesc;
	ZeroMemory(&DTexDesc, sizeof(DTexDesc));
	DTexDesc.Width          = Var.Width;
	DTexDesc.Height         = Var.Height;
	DTexDesc.MipLevels      = 1;
	DTexDesc.ArraySize      = 1;
	DTexDesc.Format         = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DTexDesc.SampleDesc     = Var.MSAA;
	DTexDesc.Usage          = D3D11_USAGE_DEFAULT;
	DTexDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
	DTexDesc.CPUAccessFlags = 0;
	DTexDesc.MiscFlags      = 0;
	Var.dev->CreateTexture2D(&DTexDesc,        NULL, &Var.DTex);
	Var.dev->CreateDepthStencilView(Var.DTex,  NULL, &Var.DSV);

	static const D3D11_VIEWPORT vp = {
		0.0f, 0.0f, (FLOAT)Var.Width, (FLOAT)Var.Height, 0.0f, 1.0f,
	};
	Var.ctx->RSSetViewports(1, &vp);

	
	//Create Instance Buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth      = sizeof(VIData) * InstanceMax;
	bd.Usage          = D3D11_USAGE_DYNAMIC;
	bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Var.dev->CreateBuffer(&bd, NULL, &Var.VInstBuf);

	// Blend
	//http://msdn.microsoft.com/ja-jp/library/ee416043(v=vs.85).aspx
	D3D11_BLEND_DESC bsd;
	memset(&bsd, 0, sizeof(bsd));
	for (int i = 0; i < 8; i++) {
		bsd.RenderTarget[i].BlendEnable           = TRUE;
		bsd.RenderTarget[i].BlendOp               = D3D11_BLEND_OP_ADD;
		bsd.RenderTarget[i].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
		bsd.RenderTarget[i].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
		bsd.RenderTarget[i].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
		bsd.RenderTarget[i].SrcBlendAlpha         = D3D11_BLEND_ONE;
		bsd.RenderTarget[i].DestBlendAlpha        = D3D11_BLEND_ZERO;
		bsd.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	Var.dev->CreateBlendState(&bsd, &Var.BS);
	
	// Set up the description of the stencil state.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable                  = TRUE;
	depthStencilDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc                    = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable                = TRUE;
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

	// Cull [Create Cull None]
	for (int i = 0; i < 2; i++) {
		D3D11_RASTERIZER_DESC rsd;
		ZeroMemory(&rsd, sizeof(rsd));
		D3D11_FILL_MODE mode[2]   = {
			D3D11_FILL_SOLID, D3D11_FILL_WIREFRAME
		};
		rsd.AntialiasedLineEnable = FALSE;
		rsd.MultisampleEnable     = TRUE;
		rsd.CullMode              = D3D11_CULL_NONE;
		//rsd.CullMode              = D3D11_CULL_BACK;
		rsd.DepthBias             = 0;
		rsd.DepthBiasClamp        = 0.0f;
		rsd.DepthClipEnable       = TRUE;
		rsd.FrontCounterClockwise = FALSE;
		rsd.ScissorEnable         = FALSE;
		rsd.SlopeScaledDepthBias  = 0.0f;
		rsd.FillMode              = mode[i];
		Var.dev->CreateRasterizerState(&rsd, &Var.RS[i]);
	}

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MipLODBias     = 0;
	sampDesc.MaxAnisotropy  = 0;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	//sampDesc.BorderColor[4];
	sampDesc.MinLOD         = 0;
	sampDesc.MaxLOD         = D3D11_FLOAT32_MAX;
	Var.dev->CreateSamplerState(&sampDesc, &Var.SState);

	D3D11_TEXTURE2D_DESC   descBackTex;
	ZeroMemory(&descBackTex, sizeof(descBackTex));
	descBackTex.Width          = Var.Width;
	descBackTex.Height         = Var.Height;
	if(Var.MSAA.Count > 1) {
		descBackTex.MipLevels      = 1;
	} else {
		descBackTex.MipLevels      = 0;
	}
	descBackTex.ArraySize      = 1;
	descBackTex.Format         = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descBackTex.SampleDesc     = Var.MSAA;
	descBackTex.Usage          = D3D11_USAGE_DEFAULT;
	descBackTex.BindFlags      = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	descBackTex.CPUAccessFlags = 0;
	descBackTex.MiscFlags      = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	D3D11_SHADER_RESOURCE_VIEW_DESC descBackSRV;
	ZeroMemory(&descBackSRV, sizeof(descBackSRV));
	Var.dev->CreateTexture2D(&descBackTex,         NULL, &Var.BackTex);
	Var.dev->CreateRenderTargetView(Var.BackTex,   NULL, &Var.BackRTV);
	
	//http://msdn.microsoft.com/ja-jp/library/ee419802%28v=vs.85%29.aspx
	Var.dev->CreateShaderResourceView(Var.BackTex, NULL, &Var.BackSRV);
	
	ResetShader();
	InitBackSwap();
	LoadPreset();
	Var.Diag();
}

void Render::LoadPreset() {
	//Create Vertex and Make rect data
	VData vertices[] = {
		{ XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f),  XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f, 1.0f,  1.0f, 1.0f),  XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f, 1.0f,  1.0f, 1.0f),  XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f, 1.0f,  1.0f, 1.0f),  XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f),  XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f, 1.0f, -1.0f, 1.0f),  XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },

		{ XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f, -1.0f,  1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f, -1.0f,  1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f, -1.0f,  1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },

		{ XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f),  XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(1.0f, -1.0f,  1.0f, 1.0f),  XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(1.0f,  1.0f,  1.0f, 1.0f),  XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(1.0f,  1.0f,  1.0f, 1.0f),  XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f),  XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(1.0f,  1.0f, -1.0f, 1.0f),  XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },

		{ XMFLOAT4(-1.0f,-1.0f,  -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f,-1.0f,   1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f, 1.0f,   1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f, 1.0f,   1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f,-1.0f,  -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f, 1.0f,  -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		
		{ XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f),  XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f,  1.0f, 1.0f, 1.0f),  XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f,  1.0f, 1.0f, 1.0f),  XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f,  1.0f, 1.0f, 1.0f),  XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f),  XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f, -1.0f, 1.0f, 1.0f),  XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },

		{ XMFLOAT4(-1.0f,  -1.0f,-1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f,   1.0f,-1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f,   1.0f,-1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f,   1.0f,-1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f,  -1.0f,-1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f,  -1.0f,-1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
	};
	Load(0, vertices, sizeof(vertices) / sizeof(vertices[0]), sizeof(VData));
}

void Render::DrawBackSwap()
{
	Var.ctx->IASetVertexBuffers(0, 1, &BackSwap.Buffer, &BackSwap.strides, &BackSwap.offsets);
	Var.ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Var.ctx->IASetInputLayout(BackSwap.IL);
	Var.ctx->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);
	
	FLOAT ColorList[4] = {1.1, 0.2, 0.3, 0.4};
	Var.ctx->OMSetRenderTargets(1, &Var.RTV, NULL);
	//Var.ctx->RSSetState(NULL);
	Var.ctx->RSSetState(Var.RS[0]);
	Var.ctx->PSSetSamplers(0, 1, &Var.SState );
	Var.ctx->PSSetShaderResources(0,  1, &Var.BackSRV);
	Var.ctx->GenerateMips(Var.BackSRV);
	Var.ctx->PSSetShaderResources(1,  1, &Var.BackSRV);
	Var.ctx->VSSetShader(BackSwap.VS,   0, NULL);
	Var.ctx->GSSetShader(NULL, 0, NULL);
	Var.ctx->PSSetShader(BackSwap.PS,   0, NULL);
	Var.ctx->Draw(BackSwap.numvertices, 0);
}


void Render::InitBackSwap() {
	VData vertices1[] = {
		{ XMFLOAT4(-1.0f,  1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f,  1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f,  1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4( 1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	};
	LPCWSTR           filename = L"swap.hlsl";
	LPCSTR            vsentry  = "vs_main";
	LPCSTR            psentry  = "ps_main";
	LPCSTR            pprofile = "ps_5_0";
	LPCSTR            vprofile = "vs_5_0";
	
	CComPtr<ID3DBlob> pBlob    = NULL;
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16 * 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16 * 1, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);
	CompileShaderFromFile(filename, vsentry, vprofile, &pBlob);
	if (pBlob) {
		Var.dev->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(),    NULL, &BackSwap.VS);
		Var.dev->CreateInputLayout(layout, numElements,        pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &BackSwap.IL);
	} else {
		printf("ERROR %s : Not found file %s", __FUNCTION__, filename);
		return;
	}

	CompileShaderFromFile(filename, psentry, pprofile, &pBlob);
	if (pBlob) Var.dev->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &BackSwap.PS);
	else       printf("ERROR Pixel Shader\n");
	
	D3D11_SUBRESOURCE_DATA subdata = {0};
	D3D11_BUFFER_DESC      bd      = {0};
	bd.ByteWidth                   = sizeof(vertices1);
	bd.Usage                       = D3D11_USAGE_DEFAULT;
	bd.BindFlags                   = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags              = 0;
	subdata.pSysMem                = vertices1;
	Var.dev->CreateBuffer(&bd, &subdata, &BackSwap.Buffer);
	BackSwap.strides     = sizeof(VData);
	BackSwap.offsets     = 0;
	BackSwap.numvertices = sizeof(vertices1) / sizeof(VData);
}

//------------------------------------------------------------------------------
// Clear
//------------------------------------------------------------------------------
void Render::Clear(FLOAT ColorList[4]) {
	Var.ctx->ClearDepthStencilView(Var.DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	Var.ctx->ClearRenderTargetView(Var.BackRTV, ColorList);
	Var.ctx->OMSetRenderTargets(1, &Var.BackRTV, Var.DSV);
}

// Swap
void Render::Swap(int sync) {
	DrawBackSwap();
	Var.chain->Present(sync, 0);
}

// Proj
void Render::Proj(float fov, float aspect, float nearz, float farz) {
	Var.matrix.Proj = XMMatrixPerspectiveFovLH(fov, aspect, nearz, farz);
}

// View
void Render::View(
	float px, float py, float pz,
	float ax, float ay, float az,
	float ux, float uy, float uz)
{
	Var.matrix.View = XMMatrixLookAtLH(
		XMLoadFloat3(&XMFLOAT3(px, py, pz)),
		XMLoadFloat3(&XMFLOAT3(ax, ay, az)),
		XMLoadFloat3(&XMFLOAT3(ux, uy, uz)));
}

// PushRect
void Render::PushRect(
	float px, float yy, float xz,
	float sx, float sy, float sz,
	float rx, float ry, float rz,
	float r, float g, float b, float a)
{
	RectBatch data = {
		px, yy, xz, 1.0,
		sx, sy, sz, 1.0,
		rx, ry, rz, 1.0,
		r, g, b, a
	};
	vRectBatch.push_back(data);
}

// Draw
void Render::Draw(int id) {
	int remain = vRectBatch.size();
	if (remain <= 0 || !Var.IL || !Var.RS || !Var.VS || !Var.PS || !Var.GS) {
		return;
	}
	int temp = 0;
	int index = 0;

	Mesh *w = &Var.mesh[id];
	if (!w->Buffer) {
		printf("%s : Not load mesh index=%d\n", __FUNCTION__, w->Buffer);
		return;
	}

	//Setup IA -> 0:Vertex, 1:Tex,Color,WorldMatrix
	ID3D11Buffer *bptr[2] = { w->Buffer, Var.VInstBuf };
	UINT strides[2]       = { w->StrideSize, sizeof(VIData) };
	UINT offsets[2]       = { 0, 0 };
	Var.ctx->IASetVertexBuffers(0, 2, bptr, strides, offsets);
	Var.ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Var.ctx->IASetInputLayout(Var.IL);
	Var.ctx->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);
	Var.ctx->RSSetState(Var.RS[0]);
	Var.ctx->VSSetShader(Var.VS, 0, NULL);
	Var.ctx->GSSetShader(Var.GS, 0, NULL);
	Var.ctx->PSSetShader(Var.PS, 0, NULL);
	XMMATRIX vp = XMMatrixMultiply(Var.matrix.View, Var.matrix.Proj);
	while (remain > 0) {
		temp = remain > InstanceMax ? InstanceMax : remain;

		D3D11_MAPPED_SUBRESOURCE m;
		if (Var.ctx->Map(Var.VInstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &m) == S_OK) {
			VIData *buffer = (VIData *)m.pData;
			RectBatch *rect = &(vRectBatch[index]);
#pragma omp parallel for
			for (int i = 0; i < temp; i++) {
				XMMATRIX rotate = XMMatrixRotationRollPitchYawFromVector(
					XMLoadFloat3(&XMFLOAT3(rect[i].rx, rect[i].ry, rect[i].rz)));
				XMMATRIX trans  = XMMatrixTranslation(rect[i].px, rect[i].py, rect[i].pz);
				XMMATRIX scale  = XMMatrixScaling(rect[i].sx, rect[i].sy, rect[i].sz);
				XMMATRIX wvp    = XMMatrixMultiply(scale, rotate);
				buffer[i].Col   = XMFLOAT4(rect[i].r, rect[i].g, rect[i].b, rect[i].a);
				buffer[i].M     = XMMatrixTranspose(
					XMMatrixMultiply(
						XMMatrixMultiply(
							XMMatrixMultiply(scale, rotate), trans), vp));
				float *w = (float *)&buffer[i].M;
			}
			Var.ctx->Unmap(Var.VInstBuf, 0);
		}

		Var.ctx->DrawInstanced(w->VertexNum, temp, 0, 0);
		remain -= temp;
		index  += temp;
	}
	printf("Debug : vRectBatch -> %d\n", vRectBatch.size());
	vRectBatch.clear();
}

void Render::Object(
    float px, float py, float pz,
    float rx, float ry, float rz,
    float sx, float sy, float sz,
    float r, float g, float b, float a)
{
	PushRect(
		px, py, pz,
		sx, sy, sz,
		rx, ry, rz,
		r, g, b, a);
}

void Render::Flip() {
	Swap(1);
	if(GetAsyncKeyState(VK_F5) & 0x8000) {
		ResetShader();
		InitBackSwap();
	}
}
