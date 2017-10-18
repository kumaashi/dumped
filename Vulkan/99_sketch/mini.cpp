#include <windows.h>

#include "vkif.h"

#pragma  comment(lib, "gdi32.lib")
#pragma  comment(lib, "winmm.lib")
#pragma  comment(lib, "user32.lib")

//#include "../include/glm/glm.hpp"
//#include "../include/glm/gtc/matrix_transform.hpp"

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

struct BinaryFile {
	std::vector<unsigned char> buf;
	size_t Size() {
		return buf.size();
	}

	BinaryFile(const char *name, const char *mode = "rb") {
		FILE *fp = fopen(name, mode);
		if (fp) {
			fseek(fp, 0, SEEK_END);
			buf.resize(ftell(fp));
			fseek(fp, 0, SEEK_SET);
			fread(&buf[0], 1, buf.size(), fp);
			fclose(fp);
		} else {
			printf("Cant Open File -> %s\n", name);
		}
	}

	void *Buf() {
		return (void *)(&buf[0]);
	}
};

void CreateProcessSync(char *szFileName) {
	PROCESS_INFORMATION pi;
	STARTUPINFO si = {};
	si.cb = sizeof(si);
	if (CreateProcess(NULL, (LPTSTR)szFileName, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		while (WaitForSingleObject(pi.hProcess, 0) != WAIT_OBJECT_0) {
			Sleep(2);
		}
	}
}

void CompileShader(const char *compilername, const char *filename, const char *optionstr, std::vector<unsigned char> &vdata) {
	//Compile Offline
	std::string infile   = std::string(filename);
	std::string outfile  = std::string(filename) + ".spv";
	std::string compiler = std::string(compilername);
	std::string option   = std::string(optionstr);
	std::string command  = compiler + " " + option + " " + outfile + " " + infile;
	printf("%s:command=%s\n", __FUNCTION__, command.c_str());
	CreateProcessSync((char *)command.c_str());
	BinaryFile file(outfile.c_str());
	vdata = file.buf;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT,
	uint64_t object,
	size_t location,
	int32_t messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* pUserData)
{
	printf("vkdbg: ");
	if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {printf("ERROR : "); }
	if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {printf("WARNING : "); }
	if(flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {printf("PERFORMANCE : "); }
	if(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {printf("INFO : "); }
	printf("%s", pMessage);
	printf("\n");
	if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {printf("\n"); }
	return VK_FALSE;
}

void BindDebugFunction(VkInstance instance) {
	VkDebugReportCallbackCreateInfoEXT callbackInfo = {};
	callbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackInfo.flags = 0;
	callbackInfo.flags |= VK_DEBUG_REPORT_ERROR_BIT_EXT;
	callbackInfo.flags |= VK_DEBUG_REPORT_WARNING_BIT_EXT;
	callbackInfo.flags |= VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	//callbackInfo.flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
	//callbackInfo.flags |= VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	 
	callbackInfo.pfnCallback = &debugCallback;

	VkDebugReportCallbackEXT callback;
	PFN_vkCreateDebugReportCallbackEXT func = PFN_vkCreateDebugReportCallbackEXT(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	
	if(func) {
		func(instance, &callbackInfo, nullptr, &callback);
	} else {
		printf("PFN_vkCreateDebugReportCallbackEXT IS NULL\n");
	}
}


int main() {
	const char *appname = "vkif";
	const uint32_t width = 1280;
	const uint32_t height = 720;
	
	//create window
	HWND hWnd = win_init(appname, width, height);
	HINSTANCE hInst = GetModuleHandle(0);
	
	//Create GPU instance 
	VulkanGpu *gpu = new VulkanGpu(appname, hWnd, hInst, width, height, BindDebugFunction);
	
	//Get BackBuffer
	std::vector<uint32_t> vBackBufferRenderPass;
	std::vector<uint32_t> vBackBufferImage;
	std::vector<uint32_t> vBackBufferDepth;

	gpu->GetBackBufferImage(vBackBufferImage);
	gpu->GetBackBufferDepth(vBackBufferDepth);
	gpu->GetBackBufferRenderPass(vBackBufferRenderPass);

	for(auto &x : vBackBufferImage) {
		printf("vBackBufferImage = %d\n", x);
	}
	for(auto &x : vBackBufferDepth) {
		printf("vBackBufferDepth = %d\n", x);
	}

	uint32_t mrt_vertex_shader     = 0;
	uint32_t mrt_fragment_shader   = 0;
	uint32_t present_vertex_shader   = 0;
	uint32_t present_fragment_shader = 0;

	const char *complier_name = "glslangValidator.exe";
	{
		const char *filehead = "basic";
		std::vector<unsigned char> vertshaderdata;
		std::vector<unsigned char> fragshaderdata;
		CompileShader(complier_name, (std::string(filehead) + ".vert").c_str(), "-V -o",  vertshaderdata);
		CompileShader(complier_name, (std::string(filehead) + ".frag").c_str(), "-V -o",  fragshaderdata);
		mrt_vertex_shader = gpu->CreateShader("main", vertshaderdata.data(), vertshaderdata.size(), VulkanGpu::ShaderData::TYPE_VERTEX);
		mrt_fragment_shader = gpu->CreateShader("main", fragshaderdata.data(), fragshaderdata.size(), VulkanGpu::ShaderData::TYPE_FRAGMENT);
	}
	{
		const char *filehead = "present";
		std::vector<unsigned char> vertshaderdata;
		std::vector<unsigned char> fragshaderdata;
		CompileShader(complier_name, (std::string(filehead) + ".vert").c_str(), "-V -o",  vertshaderdata);
		CompileShader(complier_name, (std::string(filehead) + ".frag").c_str(), "-V -o",  fragshaderdata);
		present_vertex_shader = gpu->CreateShader("main", vertshaderdata.data(), vertshaderdata.size(), VulkanGpu::ShaderData::TYPE_VERTEX);
		present_fragment_shader = gpu->CreateShader("main", fragshaderdata.data(), fragshaderdata.size(), VulkanGpu::ShaderData::TYPE_FRAGMENT);
	}
	printf("mrt_vertex_shader      =%d\n",  mrt_vertex_shader     );
	printf("mrt_fragment_shader    =%d\n",  mrt_fragment_shader   );
	printf("present_vertex_shader  =%d\n",  present_vertex_shader   );
	printf("present_fragment_shader=%d\n",  present_fragment_shader );

	static std::vector<VkVertexInputAttributeDescription> vertexInputAttr = {
		{0, 0, VK_FORMAT_R32G32B32_SFLOAT,    sizeof(float) * 0},
		{1, 0, VK_FORMAT_R32G32B32_SFLOAT,    sizeof(float) * 3},
		{2, 0, VK_FORMAT_R32G32_SFLOAT,       sizeof(float) * 6},
		{3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8},
	};

	std::vector<uint32_t> mrtShaders = {
		mrt_vertex_shader,
		mrt_fragment_shader,
	};
	std::vector<uint32_t> presentShaders = {
		present_vertex_shader,
		present_fragment_shader,
	};
	
	struct VertexFormat {
		enum {
			Position = 0,
			Normal,
			Uv,
			Color,
			Max,
		};
		float pos  [3] ;
		float nor  [3] ;
		float uv   [2] ;
		float color[4] ;
	};
	std::vector<VertexFormat> vVertex;
	VertexFormat vertex;
	vertex.pos[0] = -1.0f;
	vertex.pos[1] =  1.0f;
	vertex.pos[2] =  0.0f;
	vVertex.push_back(vertex);
	vertex.pos[0] = -1.0f;
	vertex.pos[1] = -1.0f;
	vertex.pos[2] =  0.0f;
	vVertex.push_back(vertex);
	vertex.pos[0] =  1.0f;
	vertex.pos[1] = -1.0f;
	vertex.pos[2] =  0.0f;
	vVertex.push_back(vertex);
	vertex.pos[0] = -1.0f;
	vertex.pos[1] =  1.0f;
	vertex.pos[2] =  0.0f;
	vVertex.push_back(vertex);
	vertex.pos[0] =  1.0f;
	vertex.pos[1] = -1.0f;
	vertex.pos[2] =  0.0f;
	vVertex.push_back(vertex);
	vertex.pos[0] =  1.0f;
	vertex.pos[1] =  1.0f;
	vertex.pos[2] =  0.0f;
	vVertex.push_back(vertex);
	auto handle_vertex = gpu->CreateBuffer(sizeof(VertexFormat) * 6, 0, vVertex.data());
	std::vector<uint32_t> vColorRenderTargets;
	std::vector<uint32_t> vDepthRenderTargets;
	for(int i = 0 ; i < 4; i++) {
		vColorRenderTargets.push_back(gpu->CreateImage(width, height, VulkanGpu::ImageData::TYPE_RENDERTARGET));
	}
	vDepthRenderTargets.push_back(gpu->CreateImage(width, height, VulkanGpu::ImageData::TYPE_DEPTH));
	
	uint32_t presentPipeline0 = gpu->CreatePipeline(0, presentShaders, vertexInputAttr, sizeof(VertexFormat));
	uint32_t presentPipeline1 = gpu->CreatePipeline(1, presentShaders, vertexInputAttr, sizeof(VertexFormat));
	
	uint32_t mrt_renderpass   = gpu->CreateRenderPass(vColorRenderTargets, vDepthRenderTargets, width, height);
	uint32_t mrt_pipeline     = gpu->CreatePipeline(mrt_renderpass, mrtShaders, vertexInputAttr, sizeof(VertexFormat));

	printf("mrt_renderpass  =%d\n", mrt_renderpass  );
	printf("mrt_pipeline    =%d\n", mrt_pipeline    );
	printf("presentPipeline0=%d\n", presentPipeline0);
	printf("presentPipeline1=%d\n", presentPipeline1);
	
	uint32_t index = 0;
	do {
		printf("!!!!!!! index = %d\n", index);
		uint32_t present_index = gpu->NextImage(index);
		printf("!!!!!!! present_index = %d\n", present_index);
		gpu->BeginPass(vBackBufferRenderPass[present_index % 2], 1, 1, 1, 1);
		{
			gpu->SetDescriptorSets(present_index);
			gpu->UpdateDescripterSets();
			gpu->SetPipeline(present_index, 0);
			gpu->SetViewport(present_index, width, height);
			gpu->SetVertexBuffer(present_index, handle_vertex);
			gpu->Draw(present_index, 6, 1, 0, 0);
			gpu->EndPass(present_index);
		}
		gpu->SubmitPass(present_index);
		gpu->WaitPass(present_index);
		gpu->Present(present_index);
		printf("heart\n");
		Sleep(16);
		index = (index + 1) & 1;
	} while( win_proc_msg() );
	return 0;
}

