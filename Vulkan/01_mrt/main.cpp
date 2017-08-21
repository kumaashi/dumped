#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <map>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"

#pragma  comment(lib, "gdi32.lib")
#pragma  comment(lib, "winmm.lib")
#pragma  comment(lib, "user32.lib")
#pragma  comment(lib, "vulkan-1.lib")

#include "win.h"
#include "Misc.h"

#include "util.h"

struct VertexFormat {
	enum {
		Position = 0,
		Normal,
		Uv,
		Color,
		Max,
	};
	glm::vec3 pos;
	glm::vec3 nor;
	glm::vec2 uv;
	glm::vec4 color;
};

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



float frand() {
	return float(rand()) / float(0x7FFF);
}

void CreateCubeBuffer(VertexFormat *ret)
{
	int i = 0;
	ret[i++].pos = glm::vec3(1.0, -1.0, -1.0);
	ret[i++].pos = glm::vec3(1.0, -1.0, 1.0);
	ret[i++].pos = glm::vec3(-1.0, -1.0, 1.0);
	ret[i++].pos = glm::vec3(1.0, -1.0, -1.0);
	ret[i++].pos = glm::vec3(-1.0, -1.0, 1.0);
	
	ret[i++].pos = glm::vec3(-1.0, -1.0, -1.0);
	ret[i++].pos = glm::vec3(1.0, 1.0, -1.0);
	ret[i++].pos = glm::vec3(-1.0, 1.0, -1.0);
	ret[i++].pos = glm::vec3(-1.0, 1.0, 1.0);
	ret[i++].pos = glm::vec3(1.0, 1.0, -1.0);
	
	ret[i++].pos = glm::vec3(-1.0, 1.0, 1.0);
	ret[i++].pos = glm::vec3(1.0, 1.0, 1.0);
	ret[i++].pos = glm::vec3(1.0, -1.0, -1.0);
	ret[i++].pos = glm::vec3(1.0, 1.0, -1.0);
	ret[i++].pos = glm::vec3(1.0, 1.0, 1.0);
	
	ret[i++].pos = glm::vec3(1.0, -1.0, -1.0);
	ret[i++].pos = glm::vec3(1.0, 1.0, 1.0);
	ret[i++].pos = glm::vec3(1.0, -1.0, 1.0);
	ret[i++].pos = glm::vec3(1.0, -1.0, 1.0);
	ret[i++].pos = glm::vec3(1.0, 1.0, 1.0);
	
	ret[i++].pos = glm::vec3(-1.0, 1.0, 1.0);
	ret[i++].pos = glm::vec3(1.0, -1.0, 1.0);
	ret[i++].pos = glm::vec3(-1.0, 1.0, 1.0);
	ret[i++].pos = glm::vec3(-1.0, -1.0, 1.0);
	ret[i++].pos = glm::vec3(-1.0, -1.0, 1.0);
	
	ret[i++].pos = glm::vec3(-1.0, 1.0, 1.0);
	ret[i++].pos = glm::vec3(-1.0, 1.0, -1.0);
	ret[i++].pos = glm::vec3(-1.0, -1.0, 1.0);
	ret[i++].pos = glm::vec3(-1.0, 1.0, -1.0);
	ret[i++].pos = glm::vec3(-1.0, -1.0, -1.0);
	
	ret[i++].pos = glm::vec3(1.0, 1.0, -1.0);
	ret[i++].pos = glm::vec3(1.0, -1.0, -1.0);
	ret[i++].pos = glm::vec3(-1.0, -1.0, -1.0);
	ret[i++].pos = glm::vec3(1.0, 1.0, -1.0);
	ret[i++].pos = glm::vec3(-1.0, -1.0, -1.0);
	
	ret[i++].pos = glm::vec3(-1.0, 1.0, -1.0);
	
	i = 0;
	ret[i++].nor = glm::vec3(0.0, -1.0, 0.0);
	ret[i++].nor = glm::vec3(0.0, -1.0, 0.0);
	ret[i++].nor = glm::vec3(0.0, -1.0, 0.0);
	ret[i++].nor = glm::vec3(0.0, -1.0, 0.0);
	ret[i++].nor = glm::vec3(0.0, -1.0, 0.0);
	ret[i++].nor = glm::vec3(0.0, -1.0, 0.0);
	ret[i++].nor = glm::vec3(0.0, 1.0, 0.0);
	ret[i++].nor = glm::vec3(0.0, 1.0, 0.0);
	ret[i++].nor = glm::vec3(0.0, 1.0, 0.0);
	ret[i++].nor = glm::vec3(0.0, 1.0, 0.0);
	ret[i++].nor = glm::vec3(0.0, 1.0, 0.0);
	ret[i++].nor = glm::vec3(0.0, 1.0, 0.0);
	ret[i++].nor = glm::vec3(1.0, 0.0, 0.0);
	ret[i++].nor = glm::vec3(1.0, 0.0, 0.0);
	ret[i++].nor = glm::vec3(1.0, 0.0, 0.0);
	ret[i++].nor = glm::vec3(1.0, 0.0, 0.0);
	ret[i++].nor = glm::vec3(1.0, 0.0, 0.0);
	ret[i++].nor = glm::vec3(1.0, 0.0, 0.0);
	ret[i++].nor = glm::vec3(-0.0, -0.0, 1.0);
	ret[i++].nor = glm::vec3(-0.0, -0.0, 1.0);
	ret[i++].nor = glm::vec3(-0.0, -0.0, 1.0);
	ret[i++].nor = glm::vec3(-0.0, -0.0, 1.0);
	ret[i++].nor = glm::vec3(-0.0, -0.0, 1.0);
	ret[i++].nor = glm::vec3(-0.0, -0.0, 1.0);
	ret[i++].nor = glm::vec3(-1.0, -0.0, -0.0);
	ret[i++].nor = glm::vec3(-1.0, -0.0, -0.0);
	ret[i++].nor = glm::vec3(-1.0, -0.0, -0.0);
	ret[i++].nor = glm::vec3(-1.0, -0.0, -0.0);
	ret[i++].nor = glm::vec3(-1.0, -0.0, -0.0);
	ret[i++].nor = glm::vec3(-1.0, -0.0, -0.0);
	ret[i++].nor = glm::vec3(0.0, 0.0, -1.0);
	ret[i++].nor = glm::vec3(0.0, 0.0, -1.0);
	ret[i++].nor = glm::vec3(0.0, 0.0, -1.0);
	ret[i++].nor = glm::vec3(0.0, 0.0, -1.0);
	ret[i++].nor = glm::vec3(0.0, 0.0, -1.0);
	ret[i++].nor = glm::vec3(0.0, 0.0, -1.0);
}


#define INSTANCE_MAX  1024

int main() {
	const char *appname = "vk test";
	struct UniformFormat {
		float time[4]       = {0, 0, 0, 0};
		float resolution[4] = {0, 0, 0, 0};
		float basecolor[4]  = {1, 1, 0, 1};
		float reserve[4]  = {1, 1, 0, 1};
		glm::mat4 world;
		glm::mat4 view;
		glm::mat4 proj;
		glm::vec4 instance[INSTANCE_MAX];
	};
	
	enum {
		Width          = 1280,
		Height         = 720,  //4bit -> 3840 x 1024(alignment)
		SwapImageCount = 2,
		SwapDepthCount = 1,
		MultipleRenderTargetMax = 4,
		ImageRGBA8     = 16,
		ImageRGBA32F   = 16,
		ImageMax       = ImageRGBA8 + ImageRGBA32F,
		BufferMax      = 64,
		VertexMax      = 3 * 32768,
		InstanceMax    = 1024,
		VertexCountMax = 3072, //1024poly
		MultipleRenderTargetWithDepthMax = MultipleRenderTargetMax + 1,
	};

	enum {
		CMD_UPDATE,
		CMD_RENDER,
		CMD_MAX,
	};
	
	auto hWnd  = win_init(appname, Width, Height);
	auto hInst = GetModuleHandle(NULL);
	auto instance               = new Instance(appname, hWnd, hInst, Width, Height);
	auto device                 = instance->GetDevice();
	auto queue                  = instance->GetQueue();
	auto surface                = instance->GetSurfaceKHR();
	auto queueFamilyIndex       = instance->GetQueueFamilyIndex();
	auto devicememoryprop       = instance->GetPhysicalDeviceMemoryProperties();
	auto commandPool            = new CommandPool(device, queueFamilyIndex);
	auto swapchain              = new SwapchainKHR(device, surface, Width, Height, SwapImageCount);
	auto memory_allocator       = new MemoryAllocator(device, devicememoryprop, Width * Width * ImageMax * sizeof(float) * 4, true);
	auto systemmemory_allocator = new MemoryAllocator(device, devicememoryprop, sizeof(VertexFormat) * 1024 * 1024 * 32, false);
	auto render_pass_swapchain  = new RenderPass(device, 1);

	CommandBuffer *commandBuffer[CMD_MAX] = {};
	Fence         *fence[CMD_MAX] = {};
	for(auto &x : commandBuffer) {
		x = new CommandBuffer(device, queue, commandPool->Get());
	}
	for(auto &x : fence) {
		x = new Fence(device);
	}

	//SwapChain FrameBuffer
	std::vector<Image *>       color_image_swapchain;
	std::vector<ImageView *>   color_image_view_swapchain;
	std::vector<Image *>       depth_image_swapchain;
	std::vector<ImageView *>   depth_image_view_swapchain;
	std::vector<FrameBuffer *> frame_buffer_swapchain;
	for(int i = 0 ; i < SwapImageCount; i++) {
		//Color
		auto image       = new Image(swapchain->GetImage(i), Width, Height, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
		auto depth_image = new Image(device, image->GetWidth(), image->GetHeight(), VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		memory_allocator->Bind(depth_image->Get(), depth_image->GetSize());
		auto image_view  = new ImageView(device, image->Get(), swapchain->GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
		auto depth_image_view = new ImageView(device, depth_image->Get(), depth_image->GetFormat(), VK_IMAGE_ASPECT_DEPTH_BIT);
		color_image_view_swapchain.push_back(image_view);
		depth_image_view_swapchain.push_back(depth_image_view);
		
		//FrameBuffer
		std::vector<ImageView *> vImageView;
		vImageView.push_back(image_view);
		vImageView.push_back(depth_image_view);
		auto frame_buffer = new FrameBuffer(device, render_pass_swapchain->Get(), vImageView, image->GetWidth(), image->GetHeight());
		frame_buffer_swapchain.push_back(frame_buffer);
	};
	printf("!!!!! Create RenderTarget!\n");
	//SwapChain RenderPass
	RenderPass *render_pass_4 = new RenderPass(device, MultipleRenderTargetMax, VK_FORMAT_R32G32B32A32_SFLOAT);

	//Create Render Target resource
	std::vector<Image *>       color_images;
	std::vector<Image *>       depth_images;
	std::vector<ImageView *>   color_image_views;
	std::vector<ImageView *>   depth_image_views;
	std::vector<FrameBuffer *> frame_buffers;
	std::vector<Sampler *>     samplers;

	for(int i = 0 ; i < ImageMax; i++) {
		//auto color_image = new Image(device, Width, Height, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		auto color_image = new Image(device, Width, Height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		auto depth_image = new Image(device, Width, Height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT); //VK_IMAGE_USAGE_SAMPLED_BIT
		memory_allocator->Bind(color_image->Get(), color_image->GetSize());
		memory_allocator->Bind(depth_image->Get(), depth_image->GetSize());
		auto color_image_view = new ImageView(device, color_image->Get(), color_image->GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
		auto depth_image_view = new ImageView(device, depth_image->Get(), depth_image->GetFormat(), VK_IMAGE_ASPECT_DEPTH_BIT);
		color_images.push_back(color_image);
		depth_images.push_back(depth_image);
		color_image_views.push_back(color_image_view);
		depth_image_views.push_back(depth_image_view);
		auto sampler = new Sampler(device);
		samplers.push_back(sampler);
	}
	
	//Create Render Target
	{
		auto image_view_index = 0;
		auto depth_view_index = 0;
		for(int i = 0 ; i < color_image_views.size() / MultipleRenderTargetMax; i++) {
			auto image = color_images[i]; //iikagen
			std::vector<ImageView *> vImageViews;
			for(int v = 0; v < MultipleRenderTargetMax; v++) {
				vImageViews.push_back(color_image_views[image_view_index++]);
			}
			vImageViews.push_back(depth_image_views[depth_view_index++]);
			auto frame_buffer = new FrameBuffer(device, render_pass_4->Get(), vImageViews, image->GetWidth(), image->GetHeight());
			frame_buffers.push_back(frame_buffer);
		}
	}
	
	//Create Layout Binding
	std::vector<DescriptorSetLayoutBinding> vBinding;
	vBinding.push_back(DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
	vBinding.push_back(DescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
	vBinding.push_back(DescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
	vBinding.push_back(DescriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
	vBinding.push_back(DescriptorSetLayoutBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));

	//uniform descriptor
	auto descriptorPool      = new DescriptorPool(device, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16);
	auto descriptorSetLayout = new DescriptorSetLayout(device, vBinding.size(), vBinding.data());
	auto pipelineLayout      = new PipelineLayout(device, descriptorSetLayout->Get());
	auto descriptorSet       = new DescriptorSet(device, descriptorPool->Get(), 1, descriptorSetLayout->GetAddress());

	//setup uniform 
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;

	//u0
	uint32_t binding_index = 0;
	auto uniform_buffer      = new Buffer(device, sizeof(UniformFormat));
	systemmemory_allocator->Bind(uniform_buffer->Get(), uniform_buffer->GetSize());
	auto descriptorBufferInfo = DescriptorBufferInfo(uniform_buffer->Get(), 0, uniform_buffer->GetUpdateSize());
	writeDescriptorSets.push_back(descriptorBufferInfo.GetWriteDescriptorSet(descriptorSet->Get(), binding_index));
	binding_index++;

	//s0
	auto sampler = samplers[0];
	if(true) {
		for(int i = 0 ; i < MultipleRenderTargetMax; i++) {
			auto image_view = color_image_views[i];
			writeDescriptorSets.push_back(image_view->GetWriteDescriptorSet(descriptorSet->Get(), sampler->Get(), binding_index));
			binding_index++;
		}
	}
	printf("!!!!!!!!!!!!! size and data %d %d\n", writeDescriptorSets.size(), writeDescriptorSets.data());
	vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	
	static std::vector<VkVertexInputAttributeDescription> vertexInputAttr = {
		{0, 0, VK_FORMAT_R32G32B32_SFLOAT,    sizeof(float) * 0},
		{1, 0, VK_FORMAT_R32G32B32_SFLOAT,    sizeof(float) * 3},
		{2, 0, VK_FORMAT_R32G32_SFLOAT,       sizeof(float) * 6},
		{3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8},
	};
	Pipeline *pipeline = nullptr;
	Pipeline *pipeline_present = nullptr;

	//Create Shader
	const char *complier_name = "glslangValidator.exe";
	{
		const char *filehead = "basic";
		std::vector<unsigned char> vertshaderdata;
		std::vector<unsigned char> fragshaderdata;
		CompileShader(complier_name, (std::string(filehead) + ".vert").c_str(), "-V -o",  vertshaderdata);
		CompileShader(complier_name, (std::string(filehead) + ".frag").c_str(), "-V -o",  fragshaderdata);
		auto basic_vshader = new ShaderModule(device, (const uint32_t *)vertshaderdata.data(), vertshaderdata.size(), "main");
		auto basic_fshader = new ShaderModule(device, (const uint32_t *)fragshaderdata.data(), fragshaderdata.size(), "main");
		
		//Create shader_createinfo
		std::vector<VkPipelineShaderStageCreateInfo> shader_create_info;
		shader_create_info.push_back(basic_vshader->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT));
		shader_create_info.push_back(basic_fshader->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT));
		pipeline = new Pipeline(device, vertexInputAttr, sizeof(VertexFormat), pipelineLayout->Get(), render_pass_4->Get(), render_pass_4->GetAttachmentCount(), shader_create_info);
	}

	//Create Shader
	{
		const char *filehead = "present";
		std::vector<unsigned char> vertshaderdata;
		std::vector<unsigned char> fragshaderdata;
		CompileShader(complier_name, (std::string(filehead) + ".vert").c_str(), "-V -o",  vertshaderdata);
		CompileShader(complier_name, (std::string(filehead) + ".frag").c_str(), "-V -o",  fragshaderdata);
		auto basic_vshader = new ShaderModule(device, (const uint32_t *)vertshaderdata.data(), vertshaderdata.size(), "main");
		auto basic_fshader = new ShaderModule(device, (const uint32_t *)fragshaderdata.data(), fragshaderdata.size(), "main");
		
		//Create shader_createinfo
		std::vector<VkPipelineShaderStageCreateInfo> shader_create_info;
		shader_create_info.push_back(basic_vshader->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT));
		shader_create_info.push_back(basic_fshader->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT));
		pipeline_present = new Pipeline(device, vertexInputAttr, sizeof(VertexFormat), pipelineLayout->Get(), render_pass_swapchain->Get(), render_pass_swapchain->GetAttachmentCount(), shader_create_info);
	}
	
	//Create TestBuffer
	auto *vertex_buffer = new Buffer(device, sizeof(VertexFormat) * 36);
	systemmemory_allocator->Bind(vertex_buffer->Get(), vertex_buffer->GetSize());
	void *dest = systemmemory_allocator->Map(vertex_buffer->Get());
	if(dest) {
		CreateCubeBuffer( (VertexFormat *)dest);
		systemmemory_allocator->Update(vertex_buffer->Get());
		systemmemory_allocator->Unmap();
	}
	
	//Create Rect
	auto *vertex_rect = new Buffer(device, sizeof(VertexFormat) * 6);
	{
		systemmemory_allocator->Bind(vertex_rect->Get(), vertex_rect->GetSize());
		void *dest = systemmemory_allocator->Map(vertex_rect->Get());
		if(dest) {
			VertexFormat *d = (VertexFormat *)dest;	
			int idx = 0;
			d[idx].pos[0] = -1.0f;
			d[idx].pos[1] =  1.0f;
			d[idx].pos[2] =  0.0f;
			idx++;
			d[idx].pos[0] = -1.0f;
			d[idx].pos[1] = -1.0f;
			d[idx].pos[2] =  0.0f;
			idx++;

			d[idx].pos[0] =  1.0f;
			d[idx].pos[1] = -1.0f;
			d[idx].pos[2] =  0.0f;
			idx++;

			d[idx].pos[0] = -1.0f;
			d[idx].pos[1] =  1.0f;
			d[idx].pos[2] =  0.0f;
			idx++;
			d[idx].pos[0] =  1.0f;
			d[idx].pos[1] = -1.0f;
			d[idx].pos[2] =  0.0f;
			idx++;
			d[idx].pos[0] =  1.0f;
			d[idx].pos[1] =  1.0f;
			d[idx].pos[2] =  0.0f;
			idx++;
			systemmemory_allocator->Update(vertex_rect->Get());
			systemmemory_allocator->Unmap();
		}
	}

	systemmemory_allocator->Dump();

	static float k = 0;
	UniformFormat base_data;
	for(int i = 0; i < INSTANCE_MAX; i++) {
		float x = frand() * 2.0 - 1.0;
		float y = frand() * 2.0 - 1.0;
		float z = frand() * 2.0 - 1.0;
		base_data.instance[i] = glm::vec4(x, y, z, 1.0);
	}
	while(win_proc_msg()) {
		if((GetAsyncKeyState(VK_F5) & 0x0001) == 1) {
			pipeline = nullptr;
		}
		if(pipeline == nullptr) {
			//Create Shader
			const char *complier_name = "glslangValidator.exe";
			{
				const char *filehead = "basic";
				std::vector<unsigned char> vertshaderdata;
				std::vector<unsigned char> fragshaderdata;
				CompileShader(complier_name, (std::string(filehead) + ".vert").c_str(), "-V -o",  vertshaderdata);
				CompileShader(complier_name, (std::string(filehead) + ".frag").c_str(), "-V -o",  fragshaderdata);
				auto basic_vshader = new ShaderModule(device, (const uint32_t *)vertshaderdata.data(), vertshaderdata.size(), "main");
				auto basic_fshader = new ShaderModule(device, (const uint32_t *)fragshaderdata.data(), fragshaderdata.size(), "main");
				
				//Create shader_createinfo
				std::vector<VkPipelineShaderStageCreateInfo> shader_create_info;
				shader_create_info.push_back(basic_vshader->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT));
				shader_create_info.push_back(basic_fshader->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT));
				pipeline = new Pipeline(device, vertexInputAttr, sizeof(VertexFormat), pipelineLayout->Get(), render_pass_4->Get(), render_pass_4->GetAttachmentCount(), shader_create_info);
			}

			//Create Shader
			{
				const char *filehead = "present";
				std::vector<unsigned char> vertshaderdata;
				std::vector<unsigned char> fragshaderdata;
				CompileShader(complier_name, (std::string(filehead) + ".vert").c_str(), "-V -o",  vertshaderdata);
				CompileShader(complier_name, (std::string(filehead) + ".frag").c_str(), "-V -o",  fragshaderdata);
				auto basic_vshader = new ShaderModule(device, (const uint32_t *)vertshaderdata.data(), vertshaderdata.size(), "main");
				auto basic_fshader = new ShaderModule(device, (const uint32_t *)fragshaderdata.data(), fragshaderdata.size(), "main");
				
				//Create shader_createinfo
				std::vector<VkPipelineShaderStageCreateInfo> shader_create_info;
				shader_create_info.push_back(basic_vshader->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT));
				shader_create_info.push_back(basic_fshader->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT));
				pipeline_present = new Pipeline(device, vertexInputAttr, sizeof(VertexFormat), pipelineLayout->Get(), render_pass_swapchain->Get(), render_pass_swapchain->GetAttachmentCount(), shader_create_info);
			}
			
		}
		k += 0.01;
		const float PI = 3.14159265358979323846f;
		float ffov    = PI / 90.0f;
		float faspect = (float)Width / (float)Height;
		float fnear   = 0.1;
		float ffar    = 2560.0;
		UniformFormat *dest = (UniformFormat *)systemmemory_allocator->Map(uniform_buffer->Get());
		if(dest) {
			*dest = base_data;
			dest->resolution[0] = Width;
			dest->resolution[1] = Height;
			dest->basecolor[0]  = k;
			const float radius = 20.0;
			dest->view = glm::lookAt(
				glm::vec3(radius * sin(k), -radius / 1.216f, radius * cos(k)),
				glm::vec3(0, 0, 0),
				glm::vec3(0, 1, 0));
			dest->proj = glm::perspective(glm::radians(90.0f), faspect, fnear, ffar);
			systemmemory_allocator->Unmap();
		}
		
		
		auto render_cmd = commandBuffer[CMD_RENDER];
		uint32_t present_index = 0;
		render_cmd->ResetFence();
		vkAcquireNextImageKHR(device, swapchain->Get(), UINT64_MAX, VK_NULL_HANDLE, render_cmd->GetFence(), &present_index);
		{
			auto frame_buffer = frame_buffers[0];
			auto render_pass_info = render_pass_4->GetRenderPassBeginInfo(frame_buffer->Get(), Width, Height);
			render_cmd->Reset();
			render_cmd->Update();
			render_cmd->Begin();
			render_cmd->BeginPass(&render_pass_info, Width, Height);
			render_cmd->SetPipeline(pipeline->Get());
			render_cmd->SetViewport(Width, Height);
			render_cmd->SetDescriptorSets(pipelineLayout->Get(), descriptorSet->Get());
			render_cmd->SetVertexBuffer(vertex_buffer->Get());
			render_cmd->Draw(36, INSTANCE_MAX, 0, 0);
			render_cmd->EndPass();
			render_cmd->End();
			render_cmd->Submit();
			render_cmd->Wait();
		}

		{
			auto frame_buffer     = frame_buffer_swapchain[present_index];
			auto render_pass_info = render_pass_swapchain->GetRenderPassBeginInfo(frame_buffer->Get(), Width, Height);
			render_cmd->Reset();
			render_cmd->Update();
			render_cmd->Begin();
			render_cmd->BeginPass(&render_pass_info, Width, Height);
			render_cmd->SetPipeline(pipeline_present->Get());
			render_cmd->SetViewport(Width, Height);
			render_cmd->SetDescriptorSets(pipelineLayout->Get(), descriptorSet->Get());
			render_cmd->SetVertexBuffer(vertex_rect->Get());
			render_cmd->Draw(6, 1, 0, 0);
			render_cmd->EndPass();
			render_cmd->End();
			render_cmd->Submit();
			render_cmd->Wait();
		}
		//present backbuffer
		
		VkPresentInfoKHR presentInfo = swapchain->GetPresentInfoKHR(present_index);
		render_cmd->Present(&presentInfo);
	}
	
}

