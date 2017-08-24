#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <map>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

#pragma  comment(lib, "user32.lib")
#pragma  comment(lib, "winmm.lib")
#pragma  comment(lib, "vulkan-1.lib")

#include <windows.h>
#include "util.h"

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


float frand() {
	return float(rand()) / float(0x7FFF);
}

int main() {
	const char *appname = "vk compute shader test";
	
	enum {
		Width          = 1280,
		Height         = 720,  //4bit -> 3840 x 1024(alignment)
	};

	enum {
		CMD_UPDATE,
		CMD_RENDER,
		CMD_MAX,
	};
	struct TestData {
		enum {
			Max = 256,
		};
		float data[Max];
	};
	auto hWnd  = win_init(appname, Width, Height);
	auto hInst = GetModuleHandle(NULL);
	auto instance               = new Instance(appname, hWnd, hInst, Width, Height);
	auto device                 = instance->GetDevice();
	auto queue                  = instance->GetQueue();
	auto queueFamilyIndex       = instance->GetQueueFamilyIndex();
	auto devicememoryprop       = instance->GetPhysicalDeviceMemoryProperties();
	auto commandPool            = new CommandPool(device, queueFamilyIndex);
	auto systemmemory_allocator = new MemoryAllocator(device, devicememoryprop, sizeof(TestData) * 32, false);
	auto src_buffer = new Buffer(device, sizeof(TestData),  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT );
	auto dst_buffer = new Buffer(device, sizeof(TestData),  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT );
	systemmemory_allocator->Bind(src_buffer->Get(), src_buffer->GetSize());
	systemmemory_allocator->Bind(dst_buffer->Get(), dst_buffer->GetSize());
	{
		void *dest = systemmemory_allocator->Map(src_buffer->Get());
		if(dest) {
			TestData *w = (TestData *)dest;
			for(int i = 0 ; i < TestData::Max; i++) {
				w->data[i] = float(i);
			}
			systemmemory_allocator->Unmap();
		}
	}
	auto src_descriptorBufferInfo = DescriptorBufferInfo(src_buffer->Get(), 0, src_buffer->GetUpdateSize());
	auto dst_descriptorBufferInfo = DescriptorBufferInfo(dst_buffer->Get(), 0, dst_buffer->GetUpdateSize());
	
	std::vector<DescriptorSetLayoutBinding> vBinding;
	vBinding.push_back(DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT));
	vBinding.push_back(DescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT));

	auto descriptorPool      = new DescriptorPool(device, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 16);
	auto descriptorSetLayout = new DescriptorSetLayout(device, vBinding.size(), vBinding.data());
	auto pipelineLayout      = new PipelineLayout(device, descriptorSetLayout->Get());
	auto descriptorSet       = new DescriptorSet(device, descriptorPool->Get(), 1, descriptorSetLayout->GetAddress());
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;
	uint32_t binding_index = 0;

	writeDescriptorSets.push_back(src_descriptorBufferInfo.GetWriteDescriptorSet(descriptorSet->Get(), binding_index++, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER));
	writeDescriptorSets.push_back(dst_descriptorBufferInfo.GetWriteDescriptorSet(descriptorSet->Get(), binding_index++, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER));
	vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	const char *complier_name = "glslangValidator.exe";
	const char *filehead = "basic";
	std::vector<unsigned char> compshaderdata;
	CompileShader(complier_name, (std::string(filehead) + ".comp").c_str(), "-V -o",  compshaderdata);
	auto basic_cshader = new ShaderModule(device, (const uint32_t *)compshaderdata.data(), compshaderdata.size(), "main");
	auto shader_create_info = basic_cshader->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT);
	VkComputePipelineCreateInfo cs_pipeline_create_info = {};

	cs_pipeline_create_info.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	cs_pipeline_create_info.pNext  = nullptr;
	cs_pipeline_create_info.flags  = 0;
	cs_pipeline_create_info.stage  = shader_create_info;
	cs_pipeline_create_info.layout = pipelineLayout->Get();
	VkPipeline cs_pipeline = nullptr;
	vkCreateComputePipelines(device, nullptr, 1, &cs_pipeline_create_info, nullptr, &cs_pipeline);
	CommandBuffer *compute_cmd = new CommandBuffer(device, queue, commandPool->Get(), VK_PIPELINE_BIND_POINT_COMPUTE );
	compute_cmd->Reset();
	compute_cmd->Begin();
	compute_cmd->SetPipeline(cs_pipeline);
	compute_cmd->SetDescriptorSets(pipelineLayout->Get(), descriptorSet->Get());
	compute_cmd->Dispatch(TestData::Max, 1, 1);
	compute_cmd->End();
	compute_cmd->Submit();
	compute_cmd->Wait();
	vkQueueWaitIdle(queue);
	{
		void *dest = systemmemory_allocator->Map(dst_buffer->Get());
		if(dest) {
			TestData *w = (TestData *)dest;
			for(int i = 0 ; i < TestData::Max; i++) {
				if((i % 16) == 0) printf("\n");
				printf("%8.5f, ", w->data[i]);
			}
			systemmemory_allocator->Unmap();
		}
	}
}

