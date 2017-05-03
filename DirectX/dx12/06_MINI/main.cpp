#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>
#include <map>

#include <windows.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <pix.h>

#pragma  comment(lib, "gdi32.lib")
#pragma  comment(lib, "winmm.lib")
#pragma  comment(lib, "user32.lib")
#pragma  comment(lib, "glu32.lib")
#pragma  comment(lib, "dxgi.lib")
#pragma  comment(lib, "d3d12.lib")
#pragma  comment(lib, "D3DCompiler.lib")

#ifndef RELEASE
#define RELEASE(x) {if(x) {x->Release(); x = NULL; } }
#endif

#ifndef RELEASE_MAP
#define RELEASE_MAP(cont) {for(auto & a : cont) RELEASE( (a.second) ); cont.clear(); }
#endif

static LRESULT WINAPI WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int temp = wParam & 0xFFF0;
	switch (msg) {
	case WM_SYSCOMMAND:
		if (temp == SC_MONITORPOWER || temp == SC_SCREENSAVE) {
			return 0;
		}
		break;
	case WM_IME_SETCONTEXT:
		lParam &= ~ISC_SHOWUIALL;
		return 0;
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
		break;
	case WM_SIZE:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

HWND win_init(const char *appname, int Width, int Height) {
	static WNDCLASSEX wcex = {
		sizeof(WNDCLASSEX), CS_CLASSDC | CS_VREDRAW | CS_HREDRAW,
		WindowProc, 0L, 0L, GetModuleHandle(NULL), LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH),
		NULL, appname, NULL
	};
	wcex.hInstance = GetModuleHandle(NULL);
	RegisterClassEx(&wcex);

	DWORD styleex = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD style = WS_OVERLAPPEDWINDOW;
	style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);

	RECT rc;
	SetRect( &rc, 0, 0, Width, Height );
	AdjustWindowRectEx( &rc, style, FALSE, styleex);
	rc.right -= rc.left;
	rc.bottom -= rc.top;
	HWND  hWnd = CreateWindowEx(styleex, appname, appname, style, 0, 0, rc.right, rc.bottom, NULL, NULL, wcex.hInstance, NULL);
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	ShowCursor(TRUE);
	return hWnd;
}

BOOL win_proc_msg() {
	MSG msg;
	BOOL IsActive = TRUE;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) IsActive = FALSE;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return IsActive;
}

struct Device {
	typedef std::vector<unsigned char> D3DShaderVector;

	struct Vector {
		float data[4];
	};

	struct Matrix {
		float data[16];
	};

	struct VertexFormat {
		float position[3];
		float normal[3];
		float texcoord[2];
		float color[4];
	};

	struct InstancingData {
		Matrix world;
		Vector   color;
	};

	struct MatrixData {
		Matrix world  ;
		Matrix view   ;
		Matrix proj   ;
		Matrix padding;
		Vector time;
	};

	enum {
		Width = 1280,
		Height = 720,
		MAX_BACKBUFFER = 2,
		MAX_RENDER_TARGET = 64 + MAX_BACKBUFFER,
		MAX_MULTI_RENDER_TARGET_NUM = 4, //max -> D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT
		MAX_CONSTANT_BUFFER = 32,
		MAX_TEXTURE = 256,

		MAX_RTV_HEAP = MAX_RENDER_TARGET,
		MAX_DSV_HEAP = MAX_RENDER_TARGET,
		MAX_CBV_HEAP = MAX_CONSTANT_BUFFER,
		MAX_SRV_HEAP = MAX_TEXTURE + MAX_RENDER_TARGET,

		MAX_VERTEX_BUFFER = 256,
		MAX_POLY_COUNT = 32768,
		MAX_VERTEX_COUNT = MAX_POLY_COUNT * 3,
		
		MAX_CONSTANT_BUFFER_SIZE = 32768,

		MAX_HLSL = 8,
	};

	ID3D12Resource *CreateCommittedResource(
	    UINT width,
	    UINT height = 1,
	    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN,
	    D3D12_RESOURCE_FLAGS flag = D3D12_RESOURCE_FLAG_NONE,
	    D3D12_RESOURCE_STATES resource_state = D3D12_RESOURCE_STATE_GENERIC_READ)
	{
		ID3D12Resource *ret = nullptr;
		D3D12_HEAP_PROPERTIES heap_prop  = {};
		heap_prop.Type                   = D3D12_HEAP_TYPE_DEFAULT;
		heap_prop.CPUPageProperty        = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_prop.MemoryPoolPreference   = D3D12_MEMORY_POOL_UNKNOWN;
		heap_prop.CreationNodeMask       = 1;
		heap_prop.VisibleNodeMask        = 1;

		D3D12_RESOURCE_DESC resource_desc = {};
		resource_desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resource_desc.Alignment          = 0;
		resource_desc.Width              = width;
		resource_desc.Height             = height;
		resource_desc.DepthOrArraySize   = 1;
		resource_desc.MipLevels          = 1;
		resource_desc.Format             = format;
		resource_desc.SampleDesc.Count   = 1;
		resource_desc.SampleDesc.Quality = 0;
		resource_desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resource_desc.Flags              = flag;
		if (height <= 1) {
			resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		}

		device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &resource_desc, resource_state, 0, IID_PPV_ARGS(&ret));
		return ret;
	}

	UINT64 WaitForSignalCommandComplete(ID3D12CommandQueue *queue, ID3D12Fence *fence, HANDLE fenceEvent, UINT64 fenceValue) {
		const UINT64 fenceSignalValue = fenceValue;
		queue->Signal(fence, fenceSignalValue);
		fenceValue++;
		if (fence->GetCompletedValue() < fenceSignalValue) {
			fence->SetEventOnCompletion(fenceSignalValue, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
		}
		return fenceValue;
	}

	D3D12_RESOURCE_BARRIER ResourceBarrier(
	    ID3D12Resource *resource,
	    D3D12_RESOURCE_STATES before,
	    D3D12_RESOURCE_STATES after,
	    D3D12_RESOURCE_BARRIER_TYPE type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
	{
		D3D12_RESOURCE_BARRIER ret = {};
		ret.Type = type;
		ret.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		ret.Transition.pResource = resource;
		ret.Transition.StateBefore = before;
		ret.Transition.StateAfter = after;
		ret.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		return ret;
	}

	void UploadResourceSubData(ID3D12Resource *resource, void *data, UINT size) {
		UINT8 *pData = NULL;
		resource->Map(0, NULL, reinterpret_cast<void**>(&pData));
		if (pData) {
			memcpy(pData, data, size);
			resource->Unmap(0, NULL);
		} else {
			printf("Failed Map\n");
		}
	}

	void CompileShaderFromFile(D3DShaderVector &vdest, const char *filename, const char *entryname, const char *profile, UINT flags, ID3DBlob **blobsig = 0) {
		ID3DBlob *blob = NULL;
		ID3DBlob *blobError = NULL;
		if (!filename) {
			printf("%s : Invalid Filename\n", __FUNCTION__);
			return;
		}

		std::string fstr = filename;
		std::vector<WCHAR> fname;
		for (int i = 0 ; i < fstr.length(); i++) {
			fname.push_back(filename[i]);
		}
		fname.push_back(0);

		D3DCompileFromFile(&fname[0], NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryname, profile, flags, 0, &blob, &blobError);
		if (blob) {
			vdest.resize(blob->GetBufferSize());
			memcpy(&vdest[0], blob->GetBufferPointer(), vdest.size());
		} else {
			if (blobError) {
				printf("INFO : %s\n", blobError->GetBufferPointer());
			} else {
				printf("File Not found : %s\n", filename);
			}
		}

		//https://glhub.blogspot.jp/2016/08/dx12-hlslroot-signature.html
		if (blob && blobsig) {
			D3DGetBlobPart(blob->GetBufferPointer(), blob->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0, blobsig);
		}
		RELEASE(blob);
	}

	ID3D12Device              *device      = nullptr;
	ID3D12CommandQueue        *queue       = nullptr;
	ID3D12CommandAllocator    *allocator   = nullptr;
	ID3D12GraphicsCommandList *commandlist = nullptr;
	ID3D12Fence               *fence       = nullptr;
	IDXGISwapChain3           *swapchain   = nullptr;
	IDXGIFactory4             *factory     = nullptr;
	HANDLE                     fenceEvent  = nullptr;
	UINT64                     fenceValue  = 0;
	
	int rtv_resource_index = 0;
	int dsv_resource_index = 0;
	int cbv_resource_index = 0;
	int srv_resource_index = 0;
	ID3D12DescriptorHeap *rtv_descripter_heap = nullptr;
	ID3D12DescriptorHeap *dsv_descripter_heap = nullptr;
	ID3D12DescriptorHeap *cbv_descripter_heap = nullptr;
	ID3D12DescriptorHeap *srv_descripter_heap = nullptr;
	std::map<int, ID3D12Resource *> rtv_resources;
	std::map<int, ID3D12Resource *> dsv_resources;
	std::map<int, ID3D12Resource *> cbv_resources;
	std::map<int, ID3D12Resource *> srv_resources;
	std::map<int, ID3D12Resource *> vertex_resources;
	std::map<int, ID3D12RootSignature *> root_signatures;
	std::map<int, ID3D12PipelineState *> pipeline_states;

	std::map<int, D3D12_CPU_DESCRIPTOR_HANDLE> rtv_descripter_handle;
	std::map<int, D3D12_CPU_DESCRIPTOR_HANDLE> dsv_descripter_handle;
	std::map<int, D3D12_CPU_DESCRIPTOR_HANDLE> cbv_descripter_handle;
	std::map<int, D3D12_CPU_DESCRIPTOR_HANDLE> srv_descripter_handle;

	std::map<int, D3D12_GPU_DESCRIPTOR_HANDLE> rtv_gpu_descripter_handle;
	std::map<int, D3D12_GPU_DESCRIPTOR_HANDLE> dsv_gpu_descripter_handle;
	std::map<int, D3D12_GPU_DESCRIPTOR_HANDLE> cbv_gpu_descripter_handle;
	std::map<int, D3D12_GPU_DESCRIPTOR_HANDLE> srv_gpu_descripter_handle;

	std::map<int, D3D12_VERTEX_BUFFER_VIEW> vertex_views;
	std::map<int, D3D12_CONSTANT_BUFFER_VIEW_DESC> constant_views;
	std::map<int, D3D12_SHADER_RESOURCE_VIEW_DESC> shader_views;

	D3D12_RESOURCE_BARRIER BeginBarrier[MAX_MULTI_RENDER_TARGET_NUM] = {};
	D3D12_RESOURCE_BARRIER EndBarrier[MAX_MULTI_RENDER_TARGET_NUM] = {};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvColorHandle[MAX_MULTI_RENDER_TARGET_NUM] = {};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvDepthHandle = {};
	D3D12_VIEWPORT viewport = {};
	D3D12_RECT rect = {};

	~Device() {
		RELEASE_MAP(pipeline_states);
		RELEASE_MAP(root_signatures);
		RELEASE_MAP(vertex_resources);
		RELEASE_MAP(srv_resources);
		RELEASE_MAP(cbv_resources);
		RELEASE_MAP(dsv_resources);
		RELEASE_MAP(rtv_resources);

		RELEASE(dsv_descripter_heap);
		RELEASE(rtv_descripter_heap);
		RELEASE(cbv_descripter_heap);
		RELEASE(srv_descripter_heap);
		RELEASE(swapchain);
		RELEASE(factory);
		RELEASE(fence);
		RELEASE(allocator);
		RELEASE(commandlist);
		RELEASE(queue);
		RELEASE(device);
	}

	void ReloadPipeline() {
		RELEASE_MAP(root_signatures);
		RELEASE_MAP(pipeline_states);

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			{ "POSITION",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",        0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "UV",            0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR",         0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "SV_InstanceID", 0, DXGI_FORMAT_R32_UINT,           1,  0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		};

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
		inputLayoutDesc.pInputElementDescs      = inputElementDescs;
		inputLayoutDesc.NumElements             = _countof(inputElementDescs);
		
		D3D12_RASTERIZER_DESC rasterizerState = {};
		rasterizerState.FillMode              = D3D12_FILL_MODE_SOLID;
		rasterizerState.CullMode              = D3D12_CULL_MODE_BACK;
		rasterizerState.FrontCounterClockwise = FALSE;
		rasterizerState.DepthBias             = D3D12_DEFAULT_DEPTH_BIAS;
		rasterizerState.DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizerState.SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizerState.DepthClipEnable       = TRUE;
		rasterizerState.MultisampleEnable     = FALSE;
		rasterizerState.AntialiasedLineEnable = FALSE;
		rasterizerState.ForcedSampleCount     = 0;
		rasterizerState.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		
		D3D12_DEPTH_STENCIL_DESC depthStencilState = {};
		depthStencilState.DepthEnable                  = TRUE;
		depthStencilState.DepthFunc                    = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		depthStencilState.DepthWriteMask               = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilState.StencilEnable                = FALSE;
		depthStencilState.StencilReadMask              = D3D12_DEFAULT_STENCIL_READ_MASK;
		depthStencilState.StencilWriteMask             = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		depthStencilState.FrontFace.StencilFailOp      = D3D12_STENCIL_OP_KEEP;
		depthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilState.FrontFace.StencilPassOp      = D3D12_STENCIL_OP_KEEP;
		depthStencilState.FrontFace.StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS;
		depthStencilState.BackFace.StencilFailOp       = D3D12_STENCIL_OP_KEEP;
		depthStencilState.BackFace.StencilDepthFailOp  = D3D12_STENCIL_OP_KEEP;
		depthStencilState.BackFace.StencilPassOp       = D3D12_STENCIL_OP_KEEP;
		depthStencilState.BackFace.StencilFunc         = D3D12_COMPARISON_FUNC_ALWAYS;

		D3D12_RENDER_TARGET_BLEND_DESC defaultBlendDesc = {};
		defaultBlendDesc.BlendEnable           = FALSE;
		defaultBlendDesc.LogicOpEnable         = FALSE;
		defaultBlendDesc.SrcBlend              = D3D12_BLEND_SRC_ALPHA ;
		//defaultBlendDesc.DestBlend             = D3D12_BLEND_ONE     ; //D3D12_BLEND_INV_DEST_ALPHA
		defaultBlendDesc.DestBlend             = D3D12_BLEND_INV_DEST_ALPHA;
		defaultBlendDesc.BlendOp               = D3D12_BLEND_OP_ADD;
		defaultBlendDesc.SrcBlendAlpha         = D3D12_BLEND_ONE;
		defaultBlendDesc.DestBlendAlpha        = D3D12_BLEND_ZERO;
		defaultBlendDesc.BlendOpAlpha          = D3D12_BLEND_OP_ADD;
		defaultBlendDesc.LogicOp               = D3D12_LOGIC_OP_NOOP;
		defaultBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		for(int i = 0 ; i < MAX_HLSL; i++) {
			D3DShaderVector vscode;
			D3DShaderVector gscode;
			D3DShaderVector pscode;
			ID3DBlob *blobsig = NULL;
			char shader_filename[256];
			sprintf(shader_filename, "effect%d.hlsl", i);
			CompileShaderFromFile(vscode, shader_filename, "VSMain", "vs_5_0", 0, &blobsig);
			CompileShaderFromFile(gscode, shader_filename, "GSMain", "gs_5_0", 0);
			CompileShaderFromFile(pscode, shader_filename, "PSMain", "ps_5_0", 0);
			if(!vscode.empty() && !vscode.empty()) {
				ID3D12RootSignature *signature = nullptr;
				device->CreateRootSignature(0, blobsig->GetBufferPointer(), blobsig->GetBufferSize(), IID_PPV_ARGS(&signature));
				if(signature) {
					ID3D12PipelineState *pipelinestate = nullptr;
					D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_desc = {};

					pipeline_desc.pRootSignature        = signature;
					pipeline_desc.VS.pShaderBytecode    = &vscode[0];
					pipeline_desc.VS.BytecodeLength     = vscode.size();
					if(!gscode.empty()) {
						pipeline_desc.GS.pShaderBytecode = &gscode[0];
						pipeline_desc.GS.BytecodeLength = gscode.size();
					}
					pipeline_desc.PS.pShaderBytecode    = &pscode[0];
					pipeline_desc.PS.BytecodeLength     = pscode.size();
					pipeline_desc.InputLayout           = inputLayoutDesc;
					pipeline_desc.RasterizerState       = rasterizerState;
					pipeline_desc.DepthStencilState     = depthStencilState;
					pipeline_desc.SampleMask            = UINT_MAX;
					pipeline_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
					pipeline_desc.NumRenderTargets      = MAX_MULTI_RENDER_TARGET_NUM;
					pipeline_desc.DSVFormat             = DXGI_FORMAT_D32_FLOAT;
					pipeline_desc.SampleDesc.Count      = 1;
					for (int rtvcount = 0 ; rtvcount < MAX_MULTI_RENDER_TARGET_NUM; rtvcount++) {
						pipeline_desc.RTVFormats[rtvcount] = DXGI_FORMAT_R8G8B8A8_UNORM;
					}
					pipeline_desc.BlendState.AlphaToCoverageEnable = FALSE;
					pipeline_desc.BlendState.IndependentBlendEnable = FALSE;
					for (UINT rtvcount = 0; rtvcount < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; rtvcount++) {
						pipeline_desc.BlendState.RenderTarget[rtvcount]          = defaultBlendDesc;
					}
					HRESULT status = device->CreateGraphicsPipelineState(&pipeline_desc, IID_PPV_ARGS(&pipelinestate));
					if(FAILED(status)) {
						printf("ID3D12PipelineState : cant create filename=%s\n", shader_filename);
					} else {
						root_signatures[i] = signature;
						pipeline_states[i] = pipelinestate;
					}
				}
			}
			RELEASE(blobsig);
		}
	}

	Device(HWND hWnd) {
		DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
		swap_chain_desc.BufferCount          = MAX_BACKBUFFER;
		swap_chain_desc.BufferDesc.Width     = Width;
		swap_chain_desc.BufferDesc.Height    = Height;
		swap_chain_desc.BufferDesc.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_desc.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.SwapEffect           = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swap_chain_desc.SampleDesc.Count     = 1;
		swap_chain_desc.Windowed             = TRUE;
		D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
		D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
		device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&queue));

		//misc handles
		D3D12_DESCRIPTOR_HEAP_DESC desc_heap_desc = {};
		desc_heap_desc.NumDescriptors = MAX_RTV_HEAP;
		desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		device->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(&rtv_descripter_heap));
		for(int i = 0 ; i < MAX_RTV_HEAP; i++) {
			auto handle = rtv_descripter_heap->GetCPUDescriptorHandleForHeapStart();
			auto handle_gpu = rtv_descripter_heap->GetGPUDescriptorHandleForHeapStart();
			handle.ptr += i * device->GetDescriptorHandleIncrementSize(desc_heap_desc.Type);
			handle_gpu.ptr += i * device->GetDescriptorHandleIncrementSize(desc_heap_desc.Type);
			rtv_descripter_handle[i] = handle;
			rtv_gpu_descripter_handle[i] = handle_gpu;
		}

		desc_heap_desc.NumDescriptors = MAX_DSV_HEAP;
		desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		device->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(&dsv_descripter_heap));
		for(int i = 0 ; i < MAX_DSV_HEAP; i++) {
			auto handle = dsv_descripter_heap->GetCPUDescriptorHandleForHeapStart();
			auto handle_gpu = dsv_descripter_heap->GetGPUDescriptorHandleForHeapStart();
			handle.ptr += i * device->GetDescriptorHandleIncrementSize(desc_heap_desc.Type);
			handle_gpu.ptr += i * device->GetDescriptorHandleIncrementSize(desc_heap_desc.Type);
			dsv_descripter_handle[i] = handle;
			dsv_gpu_descripter_handle[i] = handle_gpu;
		}

		desc_heap_desc.NumDescriptors = MAX_CBV_HEAP;
		desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		device->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(&cbv_descripter_heap));
		for(int i = 0 ; i < MAX_CBV_HEAP; i++) {
			auto handle = cbv_descripter_heap->GetCPUDescriptorHandleForHeapStart();
			auto handle_gpu = cbv_descripter_heap->GetGPUDescriptorHandleForHeapStart();
			handle.ptr += i * device->GetDescriptorHandleIncrementSize(desc_heap_desc.Type);
			handle_gpu.ptr += i * device->GetDescriptorHandleIncrementSize(desc_heap_desc.Type);
			cbv_descripter_handle[i] = handle;
			cbv_gpu_descripter_handle[i] = handle_gpu;
		}

		desc_heap_desc.NumDescriptors = MAX_SRV_HEAP;
		desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		device->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(&srv_descripter_heap));
		for(int i = 0 ; i < MAX_SRV_HEAP; i++) {
			auto handle = srv_descripter_heap->GetCPUDescriptorHandleForHeapStart();
			auto handle_gpu = srv_descripter_heap->GetGPUDescriptorHandleForHeapStart();
			handle.ptr += i * device->GetDescriptorHandleIncrementSize(desc_heap_desc.Type);
			handle_gpu.ptr += i * device->GetDescriptorHandleIncrementSize(desc_heap_desc.Type);
			srv_descripter_handle[i] = handle;
			srv_gpu_descripter_handle[i] = handle_gpu;
		}
		
		//SwapChain
		CreateDXGIFactory1(IID_PPV_ARGS(&factory));
		IDXGISwapChain *swapchain_temp = nullptr;
		swap_chain_desc.OutputWindow         = hWnd;
		factory->CreateSwapChain(queue, &swap_chain_desc, &swapchain_temp);
		swapchain_temp->QueryInterface( IID_PPV_ARGS( &swapchain ) );
		factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
		RELEASE(swapchain_temp);

		//BackBuffers
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		for(int i = 0 ; i < MAX_BACKBUFFER; i++) {
			ID3D12Resource *backbuffer = nullptr;
			swapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffer));
			rtv_resources[rtv_resource_index] = backbuffer;
			device->CreateRenderTargetView(rtv_resources[rtv_resource_index], &rtvDesc, rtv_descripter_handle[rtv_resource_index]);
			
			dsv_resources[dsv_resource_index] = CreateCommittedResource(Width, Height, dsvDesc.Format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
			device->CreateDepthStencilView(dsv_resources[dsv_resource_index], &dsvDesc, dsv_descripter_handle[dsv_resource_index]);
			rtv_resource_index++;
			dsv_resource_index++;
		}

		//Default Render Target
		rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = rtvDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		for(int i = rtv_resource_index ; i < MAX_RENDER_TARGET; i++) {
			rtv_resources[rtv_resource_index] = CreateCommittedResource(Width, Height, rtvDesc.Format, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
			device->CreateRenderTargetView(rtv_resources[rtv_resource_index], &rtvDesc, rtv_descripter_handle[rtv_resource_index]);
			device->CreateShaderResourceView(rtv_resources[rtv_resource_index], &srvDesc, srv_descripter_handle[rtv_resource_index]);

			dsv_resources[dsv_resource_index] = CreateCommittedResource(Width, Height, dsvDesc.Format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
			device->CreateDepthStencilView(dsv_resources[dsv_resource_index], &dsvDesc, dsv_descripter_handle[dsv_resource_index]);
			rtv_resource_index++;
			dsv_resource_index++;
		}

		//Startup
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&commandlist) );
		device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		fenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
		commandlist->Close();
		fenceValue = WaitForSignalCommandComplete(queue, fence, fenceEvent, fenceValue);

		//alloc vertex buffers
		for(int i = 0 ; i < MAX_VERTEX_BUFFER; i++) {
			auto size = MAX_VERTEX_COUNT * sizeof(VertexFormat);
			auto resource = CreateCommittedResource(size);
			vertex_resources[i] = resource;
		}

		//alloc constant buffers
		for(int i = 0 ; i < MAX_CONSTANT_BUFFER; i++) {
			D3D12_CONSTANT_BUFFER_VIEW_DESC view = {};
			const size_t SizeInBytes = (MAX_CONSTANT_BUFFER_SIZE + 255) & ~255;
			auto resource = CreateCommittedResource(SizeInBytes);
			view.SizeInBytes = SizeInBytes;
			view.BufferLocation  = resource->GetGPUVirtualAddress();
			device->CreateConstantBufferView(&view, cbv_descripter_handle[i]);
			cbv_resources[i] = resource;
			constant_views[i] = view;
		}

		ReloadPipeline();
	}

	void Begin(int *rtvIndices, size_t rtvCount, int dsvindex, float *clear_color, bool is_render_target = false) {
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width    = Width;
		viewport.Height   = Height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		rect.right = Width;
		rect.bottom = Height;
		
		for(int i = 0 ; i < rtvCount; i++) {
			int index = rtvIndices[i];
			rtvColorHandle[i] = rtv_descripter_handle[index];
			if(is_render_target) {
				BeginBarrier[i] = ResourceBarrier(rtv_resources[index], D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
				EndBarrier[i] = ResourceBarrier(rtv_resources[index], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
			} else {
				BeginBarrier[i] = ResourceBarrier(rtv_resources[index], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
				EndBarrier[i] = ResourceBarrier(rtv_resources[index], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
			}
		}
		rtvDepthHandle = dsv_descripter_handle[dsvindex];

		allocator->Reset();
		commandlist->Reset(allocator, 0);
		commandlist->RSSetViewports(1, &viewport);
		commandlist->RSSetScissorRects(1, &rect);
		commandlist->ResourceBarrier(rtvCount, BeginBarrier);
		commandlist->OMSetRenderTargets(rtvCount, rtvColorHandle, FALSE, &rtvDepthHandle);
		for (int i = 0 ; i < rtvCount; i++) {
			commandlist->ClearRenderTargetView(rtvColorHandle[i], clear_color, 0, NULL);
		}
		commandlist->ClearDepthStencilView(rtvDepthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, NULL);
		commandlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	bool SetEffect(int index) {
		if(pipeline_states[index]) {
			commandlist->SetPipelineState(pipeline_states[index]);
			commandlist->SetGraphicsRootSignature(root_signatures[index]);
			return true;
		}
		return false;
	}

	void SetVertex(int index) {
		D3D12_VERTEX_BUFFER_VIEW vb_view = vertex_views[index];
		commandlist->IASetVertexBuffers(0, 1, &vb_view);
	}

	void SetConstant(int tableindex, int index) {
		commandlist->SetDescriptorHeaps(1, &cbv_descripter_heap);
		commandlist->SetGraphicsRootDescriptorTable(tableindex, cbv_gpu_descripter_handle[index]);
	}

	void SetTexture(int tableindex, int index) {
		commandlist->SetDescriptorHeaps(1, &srv_descripter_heap);
		commandlist->SetGraphicsRootDescriptorTable(tableindex, srv_gpu_descripter_handle[index]);
	}

	void End(int rtvCount) {
		commandlist->ResourceBarrier(rtvCount, EndBarrier);
		commandlist->Close();
		ID3D12CommandList *ppLists[] = {
			commandlist,
		};
		queue->ExecuteCommandLists(1, ppLists);
		fenceValue = WaitForSignalCommandComplete(queue, fence, fenceEvent, fenceValue);
	}
	
	void Present() {
		swapchain->Present(1, 0);
	}

	void UploadConstant(int index, void *data, size_t size) {
		auto res = cbv_resources[index];
		UploadResourceSubData(res, data, size);
	}

	void UploadVertex(int index, void *data, size_t size,  size_t stride_size) {
		D3D12_VERTEX_BUFFER_VIEW view = {};
		auto resource = vertex_resources[index];
		view.StrideInBytes = stride_size;
		view.SizeInBytes = size;
		view.BufferLocation = resource->GetGPUVirtualAddress();
		UploadResourceSubData(resource, data, size);
		vertex_views[index] = view;
	}

};

Device::Matrix GetTrans(float x, float y, float z) {
	using namespace DirectX;
	DirectX::XMMATRIX data = XMMatrixTranslation(x, y, z);
	return *(Device::Matrix *)&data;
}

Device::Matrix GetView(float px, float py, float pz,
                          float ax, float ay, float az,
                          float ux, float uy, float uz)
{
	using namespace DirectX;
	XMVECTOR vpos = XMVectorSet(px, py, pz, 1.0);
	XMVECTOR vat = XMVectorSet(ax, ay, az, 1.0);
	XMVECTOR vup = XMVectorSet(ux, uy, uz, 1.0);
	DirectX::XMMATRIX data = XMMatrixLookAtLH(vpos, vat, vup);
	return *(Device::Matrix *)&data;
}

Device::Matrix GetProj(float FovAngleY, float AspectRatio, float NearZ, float FarZ) {
	using namespace DirectX;
	DirectX::XMMATRIX data = XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ);
	return *(Device::Matrix *)&data;
}

unsigned long random() {
	static unsigned long a = 1;
	static unsigned long b = 2345678;
	static unsigned long c = 9012345;
	a += b;
	b += c;
	c += a;
	return(a >> 16);
}

float frandom() {
	return float(random()) / float(0xFFFF);
}

float frandom2() {
	return frandom() * 2.0 - 1.0;
}

int main() {
	auto hWnd = win_init("minidx12", Device::Width, Device::Height);
	Device device(hWnd);

	//setup default param
	std::vector<Device::VertexFormat> vbuffer;
	float cube_vertex[] = {
		1.0, -1.0, -1.0, 1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 
		1.0, -1.0, -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, -1.0, 
		1.0, 1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 
		1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
		1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 
		1.0, -1.0, -1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 
		1.0, -1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 
		1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 
		-1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 
		-1.0, -1.0, 1.0, -1.0, 1.0, -1.0, -1.0, -1.0, -1.0, 
		1.0, 1.0, -1.0, 1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 
		1.0, 1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 1.0, -1.0, 
	};

	for(int vcount = 0 ; vcount < _countof(cube_vertex); vcount += 3) {
		Device::VertexFormat data;
		data.position[0] = cube_vertex[vcount + 0];
		data.position[1] = cube_vertex[vcount + 1];
		data.position[2] = cube_vertex[vcount + 2];
		vbuffer.push_back(data);
	}
	device.UploadVertex(0, &vbuffer[0], vbuffer.size() * sizeof(Device::VertexFormat), sizeof(Device::VertexFormat));

	int present_count = 0;
	
	std::vector<Device::InstancingData> instdata(256);
	for(int i = 0 ; i < 256; i++) {
		Device::InstancingData *p = &instdata[i];
		p->world = GetTrans(frandom2() * 3.0, frandom2() * 3.0, frandom2() * 3.0);
		float scale = 0.1;
		p->world.data[0] = scale;
		p->world.data[5] = scale;
		p->world.data[10] = scale;
		p->color.data[0] = frandom();
		p->color.data[1] = frandom();
		p->color.data[2] = frandom();
		p->color.data[3] = 1.0;
	}
	std::vector<int> rendertarget_indices;
	for(int i = 0 ; i < 4; i++) {
		rendertarget_indices.push_back(Device::MAX_BACKBUFFER + i);
	}
	while(win_proc_msg()) {
		if(GetAsyncKeyState(VK_F5) & 0x0001) {
			device.ReloadPipeline();
		}

		Device::MatrixData matrixdata;
		
		matrixdata.view = GetView(
			cos(present_count * 0.01) * 3.0, 2, sin(present_count * 0.01) * 3.0,
			0, 0, 0,
			0, 1, 0);
		matrixdata.proj = GetProj(3.141592 / 2.0, float(Device::Width) / float(Device::Height), 0.1f, 256.0f);
		matrixdata.time.data[0] = float(present_count) * 1.0f / 60.0f;

		if(true)
		{
			int matrixdata_index = 0;
			int instancingdata_index = 1;
			device.UploadConstant(matrixdata_index, &matrixdata, sizeof(matrixdata));
			device.UploadConstant(instancingdata_index, &instdata[0], instdata.size() * sizeof(Device::InstancingData));

			float clear_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f, };
			device.Begin(&rendertarget_indices[0], rendertarget_indices.size(), rendertarget_indices[0], clear_color, true);
			if(device.SetEffect(0)) {
				device.SetVertex(0);
				device.SetConstant(0, matrixdata_index);
				device.SetConstant(1, instancingdata_index);
				device.commandlist->DrawInstanced(_countof(cube_vertex), instdata.size(), 0, 0);
			}
			device.End(rendertarget_indices.size());
		}

		{
			int matrixdata_index = 2;
			int instancingdata_index = 3;
			device.UploadConstant(matrixdata_index, &matrixdata, sizeof(matrixdata));
			device.UploadConstant(instancingdata_index, &instdata[0], instdata.size() * sizeof(Device::InstancingData));
            
			float clear_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f, };
			int rtvCount = 1;
			int swap_index = present_count % Device::MAX_BACKBUFFER;
			device.Begin(&swap_index, rtvCount, swap_index, clear_color);
			if(device.SetEffect(1)) {
				device.SetVertex(0);
				device.SetConstant(0, matrixdata_index);
				device.SetConstant(1, instancingdata_index);
				device.SetTexture(2, rendertarget_indices[0]);
				device.SetTexture(3, rendertarget_indices[1]);
				device.SetTexture(4, rendertarget_indices[2]);
				device.SetTexture(5, rendertarget_indices[3]);
				device.commandlist->DrawInstanced(_countof(cube_vertex), 1, 0, 0);
			}
			device.End(rtvCount);
		}
		device.Present();
		present_count++;
	}
	

	return 0;
}


