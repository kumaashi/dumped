#pragma once

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <map>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

#pragma  comment(lib, "gdi32.lib")
#pragma  comment(lib, "winmm.lib")
#pragma  comment(lib, "user32.lib")
#pragma  comment(lib, "vulkan-1.lib")

#pragma warning(disable:4477)
#pragma warning(disable:4313)

class VulkanGpu {
public:
	struct BufferData {
		VkBuffer buffer = nullptr;
		VkBufferCreateInfo info = {};
		VkMemoryRequirements memreq = {};
	};

	struct ImageData {
		enum {
			TYPE_BACKBUFFER   = 1 << 0,
			TYPE_TEXTURE      = 1 << 1,
			TYPE_RENDERTARGET = 1 << 2,
			TYPE_DEPTH        = 1 << 3,
			TYPE_MAX          = 1 << 31,
		};
		VkImage image = nullptr;
		VkImageView view = nullptr;
		VkImageCreateInfo info = {};
		VkMemoryRequirements memreq = {};
		VkDescriptorImageInfo desc_image_info = {};
		bool isbackbuffer = false;
	};

	struct SamplerData {
		enum {
			TYPE_NEARST,
			TYPE_LINEAR,
			TYPE_MAX          = 1 << 31,
		};
		VkSampler sampler = nullptr;
	};

	struct ShaderData {
		enum {
			TYPE_VERTEX,
			TYPE_TESSELLATION_CONTROL,
			TYPE_TESSELLATION_EVALUATION,
			TYPE_GEOMETRY,
			TYPE_FRAGMENT,
			TYPE_COMPUTE,
			TYPE_MAX,
		};
		uint32_t type = TYPE_VERTEX;
		VkShaderModule shader = nullptr;
		std::string entrypoint;
	};

	struct PipelineData {
		VkPipeline pipeline = nullptr;
	};

	struct DeviceMemoryData {
		enum {
			TYPE_LOCAL   = 1 << 0,
			TYPE_HOST    = 1 << 1,
			TYPE_MAX     = 1 << 31,
		};
		VkDeviceMemory devicememory = nullptr;
		VkDeviceSize   offset = 0;
	};

	struct RenderPassData {
		VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
		VkRenderPass renderpass = nullptr;
		VkFramebuffer framebuffer = nullptr;
		VkCommandBuffer commandbuffer = nullptr;
		VkFence fence = nullptr;
		std::vector<uint32_t> color_attachments;
		std::vector<uint32_t> depth_attachments;
		uint32_t width = 0;
		uint32_t height = 0;
	};
private:
	enum {
		QUEUE_TYPE_GRAPHICS,
		QUEUE_TYPE_COMPUTE,
		QUEUE_TYPE_MAX,
	};

	enum {
		MEMORY_TYPE_LOCAL,
		MEMORY_TYPE_HOST,
		MEMORY_TYPE_MAX,
	};
	
	std::map<uint32_t, BufferData> vBuffer;
	uint32_t BufferIndex = 0;

	std::map<uint32_t, ImageData>  vImage;
	uint32_t ImageIndex = 0;
	
	std::map<uint32_t, RenderPassData> vRenderPass;
	uint32_t RenderPassIndex = 0;

	std::map<uint32_t, ShaderData> vShader;
	uint32_t ShaderIndex = 0;
	
	std::map<uint32_t, PipelineData> vPipeline;
	uint32_t PipelineIndex = 0;
	
	std::map<uint32_t, SamplerData> vSampler;
	uint32_t SamplerIndex = 0;
	

	std::map<uint32_t, DeviceMemoryData> vDeviceMemory;

	VkInstance instance;
	VkSurfaceKHR surface;
	std::vector<VkPhysicalDevice> adapters;
	VkPhysicalDevice phydevice = nullptr;
	VkDevice device = nullptr;
	VkQueue queue = nullptr;
	VkCommandPool commandpool = nullptr;
	VkSwapchainKHR swapchain;
	
	VkPhysicalDeviceFeatures devicefeatures;
	VkPhysicalDeviceProperties deviceprop;
	VkPhysicalDeviceMemoryProperties devicememoryprop;
	VkDescriptorPool descPool = nullptr;
	VkDescriptorSetLayout descSetLayout = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	VkDescriptorSet descSet = nullptr;
	uint32_t queueFamilyIndex = 0xffffffff;
	uint32_t swapchainCount = 0;
	uint32_t adapterCount = 0;

	//backbuffer info
	std::vector<uint32_t> backbuffer_image;
	std::vector<uint32_t> backbuffer_depth;
	std::vector<uint32_t> backbuffer_renderpass;
	
	uint32_t dummy_image_handle = 0;
public:
	void GetBackBufferImage(std::vector<uint32_t> &dest) {
		dest = backbuffer_image;
	}

	void GetBackBufferDepth(std::vector<uint32_t> &dest) {
		dest = backbuffer_depth;
	}

	void GetBackBufferRenderPass(std::vector<uint32_t> &dest) {
		dest = backbuffer_renderpass;
	}
	
	VulkanGpu(const char *appname, HWND hWnd, HINSTANCE hInst, uint32_t width, uint32_t height, void (*instance_cb)(VkInstance) = nullptr) {
		//memset(this, 0, sizeof(*this));

		static const char *layers[] = { "VK_LAYER_LUNARG_standard_validation" };
		static const char *extensions[] = { "VK_KHR_swapchain" };
		static const char* extensions_info[] = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		};

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pApplicationName   = appname;
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instanceInfo = {};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledExtensionCount = std::size(extensions_info);
		instanceInfo.ppEnabledExtensionNames = extensions_info;
		instanceInfo.enabledLayerCount = std::size(layers);
		instanceInfo.ppEnabledLayerNames = layers;
		vkCreateInstance(&instanceInfo, nullptr, &instance);
		if(instance_cb) {
			instance_cb(instance);
			//BindDebugFunction(GetInstance());
		}

		//Create PhyDevice
		vkEnumeratePhysicalDevices(instance, &adapterCount, NULL);
		adapters.resize(adapterCount);
		vkEnumeratePhysicalDevices(instance, &adapterCount, adapters.data());

		//default
		phydevice = adapters[0];

		//Create Surface
		VkWin32SurfaceCreateInfoKHR surfaceinfo = {};
		surfaceinfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceinfo.hinstance = hInst;
		surfaceinfo.hwnd      = hWnd;
		vkCreateWin32SurfaceKHR(instance, &surfaceinfo, NULL, &surface);
		
		//Get Prop
		uint32_t surfaceFormatCount = 0;
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(phydevice, surface, &surfaceFormatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> vSurfaceFormatKHR(surfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(phydevice, surface, &surfaceFormatCount, vSurfaceFormatKHR.data());
		for(size_t i = 0; i < vSurfaceFormatKHR.size(); i++) {
			VkSurfaceFormatKHR fmt = vSurfaceFormatKHR[i];
		}
	    vkGetPhysicalDeviceSurfacePresentModesKHR(phydevice, surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> vPresentModeKHR(presentModeCount);
	    vkGetPhysicalDeviceSurfacePresentModesKHR(phydevice, surface, &presentModeCount, vPresentModeKHR.data());
		vkGetPhysicalDeviceMemoryProperties(phydevice, &devicememoryprop);

		uint32_t propertyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(phydevice, &propertyCount, nullptr);
		std::vector<VkQueueFamilyProperties> properties(propertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(phydevice, &propertyCount, &properties[0]);
		auto usageFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
		for (uint32_t i = 0; i < propertyCount; i++) {
			printf("%08X\n", properties[i].queueFlags);
			//VK_QUEUE_GRAPHICS_BIT = 0x00000001,
			//VK_QUEUE_COMPUTE_BIT = 0x00000002,
			//VK_QUEUE_TRANSFER_BIT = 0x00000004,
			//VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008,
			auto flags = properties[i].queueFlags;
			if ( (flags & usageFlags) == usageFlags) {
				printf("queueFamilyIndex = %d\n", i);
				queueFamilyIndex = i;
				break;
			}
		}
		

		//Create Device
		VkDeviceQueueCreateInfo queueInfo = {};
		static float qPriorities[] = { 0.0f };
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueCount = 1;
		queueInfo.queueFamilyIndex = queueFamilyIndex;
		queueInfo.pQueuePriorities = qPriorities;

		VkDeviceCreateInfo devInfo = {};
		devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		devInfo.queueCreateInfoCount = 1;
		devInfo.pQueueCreateInfos = &queueInfo;
		devInfo.enabledLayerCount = std::size(layers);
		devInfo.ppEnabledLayerNames = layers;
		devInfo.enabledExtensionCount = std::size(extensions);
		devInfo.ppEnabledExtensionNames = extensions;
		vkCreateDevice(phydevice, &devInfo, NULL, &device);
		vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(phydevice, 0, surface, &presentSupport);
		VkSurfaceCapabilitiesKHR capabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phydevice, surface, &capabilities);

		//Create Device Memory
		{
			VkDeviceSize allocsize = 1024 * 1024 * 1024;
			VkMemoryPropertyFlags memorypropflags[MEMORY_TYPE_MAX] = {
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			};
			for(int i = 0; i < MEMORY_TYPE_MAX; i++) {
				VkMemoryPropertyFlags flags = memorypropflags[i];
				VkMemoryAllocateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				info.allocationSize = allocsize;
				for (uint32_t i = 0; i < devicememoryprop.memoryTypeCount; i++) {
					if ((devicememoryprop.memoryTypes[i].propertyFlags & flags) == flags) {
						info.memoryTypeIndex = i;
						break;
					}
				}
				VkDeviceMemory devicememory = nullptr;
				vkAllocateMemory(device, &info, nullptr, &devicememory);
				printf("vkAllocateMemory=%016X\n", devicememory);

				DeviceMemoryData data = {};
				data.devicememory = devicememory;
				data.offset = 0;
				vDeviceMemory[i] = data;
			}
		}

		//Create Command Pool
		{
			VkCommandPoolCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.pNext = NULL;
			info.queueFamilyIndex = queueFamilyIndex;
			info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			vkCreateCommandPool(device, &info, NULL, &commandpool);
		}

		//Create Swapchain and render pass
		{
			VkSwapchainCreateInfoKHR info = {};
			info.sType              = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			info.surface            = surface;
			info.minImageCount      = 2;
			info.imageFormat        = VK_FORMAT_B8G8R8A8_UNORM;
			info.imageColorSpace    = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
			info.imageExtent.width  = width;
			info.imageExtent.height = height;
			info.imageArrayLayers   = 1;
			info.imageUsage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			info.imageSharingMode   = VK_SHARING_MODE_EXCLUSIVE;
			info.queueFamilyIndexCount = 0; //VK_SHARING_MODE_CONCURRENT
			info.pQueueFamilyIndices = nullptr;   //VK_SHARING_MODE_CONCURRENT
			info.preTransform       = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ; //http://vulkan-spec-chunked.ahcox.com/ch29s05.html#VkSurfaceTransformFlagBitsKHR
			info.compositeAlpha     = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			info.preTransform       = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			info.presentMode        = VK_PRESENT_MODE_FIFO_KHR;
			info.clipped            = VK_TRUE;
			info.oldSwapchain       = VK_NULL_HANDLE;
			vkCreateSwapchainKHR(device, &info, nullptr, &swapchain);
			uint32_t count = 0;
			vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
			std::vector<VkImage> images(count);
			vkGetSwapchainImagesKHR(device, swapchain, &count, images.data());
			for(int i = 0 ; i < count; i++) {
				uint32_t handle_image = CreateImage(width, height, ImageData::TYPE_BACKBUFFER, images[i]);
				uint32_t handle_depth = CreateImage(width, height, ImageData::TYPE_DEPTH);
				backbuffer_image.push_back(handle_image);
				backbuffer_depth.push_back(handle_depth);
				std::vector<uint32_t> vImage = {handle_image};
				std::vector<uint32_t> vDepth = {handle_depth};
				uint32_t renderpass_handle = CreateRenderPass(vImage, vDepth, width, height);
				backbuffer_renderpass.push_back(renderpass_handle);
			}
		}
		
		//Create Dummy Image
		{
			dummy_image_handle = CreateImage(256, 256, ImageData::TYPE_TEXTURE, nullptr);
		}
		
		//Create DescPool
		{
			const uint32_t countValue = 32;
			const uint32_t maxSets = 32;

			std::vector<VkDescriptorPoolSize> vPoolSize;
			vPoolSize.push_back({VK_DESCRIPTOR_TYPE_SAMPLER, countValue});
			vPoolSize.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, countValue});
			vPoolSize.push_back({VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, countValue});
			vPoolSize.push_back({VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, countValue});
			vPoolSize.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, countValue});
			vPoolSize.push_back({VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, countValue});
			vPoolSize.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, countValue});
			vPoolSize.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, countValue});
			vPoolSize.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, countValue});
			vPoolSize.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, countValue});
			vPoolSize.push_back({VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, countValue});
			VkDescriptorPoolCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			info.poolSizeCount = vPoolSize.size();
			info.pPoolSizes = vPoolSize.data();
			info.maxSets = maxSets;
			vkCreateDescriptorPool(device, &info, nullptr, &descPool);
			printf("vkCreateDescriptorPool=%016X\n", descPool);
		}

		//Create Desc Layout
		{
			std::vector<VkDescriptorSetLayoutBinding> vBinding;
			vBinding.push_back({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr});
			vBinding.push_back({1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr});
			vBinding.push_back({2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr});
			vBinding.push_back({3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr});
			vBinding.push_back({4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr});

			VkDescriptorSetLayoutCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.pBindings = vBinding.data();
			info.bindingCount = vBinding.size();
			vkCreateDescriptorSetLayout(device, &info, nullptr, &descSetLayout);
			printf("vkCreateDescriptorSetLayout=%016X\n", descSetLayout);
		}

		//create pipelineLayout
		{
			VkPipelineLayoutCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.pNext = nullptr;
			info.setLayoutCount = 1;
			info.pSetLayouts = &descSetLayout;
			vkCreatePipelineLayout(device, &info, nullptr, &pipelineLayout);
			printf("vkCreatePipelineLayout=%016X\n", pipelineLayout);
		}

		//Create DescriptorSet
		{
			VkDescriptorSetAllocateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.descriptorPool = descPool;
			info.descriptorSetCount = 1;
			info.pSetLayouts = &descSetLayout;
			vkAllocateDescriptorSets(device, &info, &descSet);
			printf("vkAllocateDescriptorSets=%016X\n", descSet);
		}

		//Create ConstantSampler
		{
			for(int i = 0 ; i < 2; i++) {
				VkSampler sampler = nullptr;
				VkSamplerCreateInfo sampleInfo = {};
				sampleInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				sampleInfo.magFilter = VK_FILTER_NEAREST;
				sampleInfo.minFilter = VK_FILTER_NEAREST;
				sampleInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				sampleInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				sampleInfo.addressModeV = sampleInfo.addressModeU;
				sampleInfo.addressModeW = sampleInfo.addressModeU;
				sampleInfo.mipLodBias = 0.0f;
				sampleInfo.anisotropyEnable = VK_FALSE;
				sampleInfo.maxAnisotropy = 1.0f;
				sampleInfo.minLod = 0.0f;
				sampleInfo.maxLod = 1.0f;
				sampleInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
				vkCreateSampler(device, &sampleInfo, nullptr, &sampler);
				SamplerData data;
				data.sampler = sampler;
				uint32_t ret = SamplerIndex++;
				vSampler[ret] = data;
			}
		}

		//WriteDesc
		{
			std::vector<VkWriteDescriptorSet> writeDescriptorSets;
			std::vector<VkDescriptorBufferInfo> desc_buffer_infos(64);
			std::vector<VkDescriptorImageInfo> desc_image_infos(64);
			uint32_t binding_index = 0;
			{
				size_t uniform_size = 32768;
				uint32_t handle = CreateBuffer(32768);
				VkDescriptorBufferInfo *descriptorbufferinfo = &desc_buffer_infos[binding_index];
				descriptorbufferinfo->buffer = vBuffer[handle].buffer;
				descriptorbufferinfo->offset = 0;
				descriptorbufferinfo->range  = uniform_size;
				VkWriteDescriptorSet writedescriptorset = {};
				writedescriptorset.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writedescriptorset.dstSet = descSet;
				writedescriptorset.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writedescriptorset.dstBinding = binding_index;
				writedescriptorset.descriptorCount = 1;
				writedescriptorset.pBufferInfo = descriptorbufferinfo;
				writeDescriptorSets.push_back(writedescriptorset);
				binding_index++;
			}

			//s0
			auto sampler = vSampler[0];
			auto dummy_image = vImage[dummy_image_handle];
			for(int i = 0 ; i < 4; i++) {
				VkDescriptorImageInfo *descriptorimageinfo = &desc_image_infos[binding_index];
				*descriptorimageinfo = dummy_image.desc_image_info;
				descriptorimageinfo->sampler = sampler.sampler;
				VkWriteDescriptorSet writedescriptorset = {};
				writedescriptorset.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writedescriptorset.dstSet = descSet;
				writedescriptorset.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writedescriptorset.dstBinding = binding_index;
				writedescriptorset.descriptorCount = 1;
				writedescriptorset.pImageInfo = descriptorimageinfo;

				writeDescriptorSets.push_back(writedescriptorset);
				binding_index++;
			}
			vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	~VulkanGpu() {
	}

	uint32_t CreatePipeline(uint32_t pass, std::vector<uint32_t> &shaders, std::vector<VkVertexInputAttributeDescription> &vertexInputAttr, VkDeviceSize strideSize) {
		VkVertexInputBindingDescription vertex_input_binding_desc = {};
		vertex_input_binding_desc.binding = 0;
		vertex_input_binding_desc.stride = strideSize;
		vertex_input_binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		
		VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info = {};
		pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
		pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_input_binding_desc;
		pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = vertexInputAttr.size();
		pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = vertexInputAttr.data();
		
		VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info = {};
		pipeline_input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		
		VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info = {};
		pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipeline_rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline_rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE; //VK_CULL_MODE_BACK_BIT;
		pipeline_rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		pipeline_rasterization_state_create_info.lineWidth = 1.0f;
		pipeline_rasterization_state_create_info.depthClampEnable = VK_FALSE;
		
		VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info = {};
		pipeline_depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipeline_depth_stencil_state_create_info.depthTestEnable = VK_TRUE;
		pipeline_depth_stencil_state_create_info.depthWriteEnable = VK_TRUE;
		pipeline_depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		pipeline_depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
		pipeline_depth_stencil_state_create_info.back.failOp = VK_STENCIL_OP_KEEP;
		pipeline_depth_stencil_state_create_info.back.passOp = VK_STENCIL_OP_KEEP;
		pipeline_depth_stencil_state_create_info.back.compareOp = VK_COMPARE_OP_ALWAYS;
		pipeline_depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
		pipeline_depth_stencil_state_create_info.front = pipeline_depth_stencil_state_create_info.back;
		
		VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info = {};
		pipeline_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipeline_viewport_state_create_info.viewportCount = 1;
		pipeline_viewport_state_create_info.scissorCount  = 1;
		
		VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info = {};
		pipeline_multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipeline_multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		
		std::vector<VkDynamicState> vDynamicStates;
		VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info = {};
		vDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		vDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
		pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		pipeline_dynamic_state_create_info.pDynamicStates = vDynamicStates.data();
		pipeline_dynamic_state_create_info.dynamicStateCount = vDynamicStates.size();

		
		std::vector<VkPipelineColorBlendAttachmentState> v_pipeline_color_blend_attachment_state;
		VkBool32 blendEnable = VK_FALSE;
		auto passdata = vRenderPass[pass];
		for(int i = 0 ; i < passdata.color_attachments.size(); i++) {
			VkPipelineColorBlendAttachmentState temp = {};
			temp.colorWriteMask = 0xF;
			temp.blendEnable    = blendEnable;
			v_pipeline_color_blend_attachment_state.push_back(temp);
		}
		VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info = {};
		pipeline_color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipeline_color_blend_state_create_info.attachmentCount = v_pipeline_color_blend_attachment_state.size();
		pipeline_color_blend_state_create_info.pAttachments    = v_pipeline_color_blend_attachment_state.data();
		
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		for(auto &index : shaders) {
			auto shader = vShader[index];
			VkShaderStageFlagBits flag = VK_SHADER_STAGE_VERTEX_BIT;
			if(shader.type == ShaderData::TYPE_VERTEX)  flag = VK_SHADER_STAGE_VERTEX_BIT;
			if(shader.type == ShaderData::TYPE_TESSELLATION_CONTROL)  flag = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			if(shader.type == ShaderData::TYPE_TESSELLATION_EVALUATION)  flag = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			if(shader.type == ShaderData::TYPE_GEOMETRY)  flag = VK_SHADER_STAGE_GEOMETRY_BIT;
			if(shader.type == ShaderData::TYPE_FRAGMENT)  flag = VK_SHADER_STAGE_FRAGMENT_BIT;
			if(shader.type == ShaderData::TYPE_COMPUTE)  flag = VK_SHADER_STAGE_COMPUTE_BIT;
			VkPipelineShaderStageCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.stage = flag;
			info.module = shader.shader;
			info.pName = shader.entrypoint.c_str();
			shaderStages.push_back(info);
		}
		VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {};
		graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphics_pipeline_create_info.stageCount          = shaderStages.size();
		graphics_pipeline_create_info.pStages             = shaderStages.data();
		graphics_pipeline_create_info.pVertexInputState   = &pipeline_vertex_input_state_create_info;
		graphics_pipeline_create_info.pInputAssemblyState = &pipeline_input_assembly_state_create_info;
		graphics_pipeline_create_info.pTessellationState  = nullptr;
		graphics_pipeline_create_info.pViewportState      = &pipeline_viewport_state_create_info;
		graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
		graphics_pipeline_create_info.pMultisampleState   = &pipeline_multisample_state_create_info;
		graphics_pipeline_create_info.pDepthStencilState  = &pipeline_depth_stencil_state_create_info;
		graphics_pipeline_create_info.pColorBlendState    = &pipeline_color_blend_state_create_info;
		graphics_pipeline_create_info.pDynamicState       = &pipeline_dynamic_state_create_info;
		graphics_pipeline_create_info.layout              = pipelineLayout;
		graphics_pipeline_create_info.renderPass          = passdata.renderpass;
		graphics_pipeline_create_info.subpass             = 0;

		const uint32_t createInfoCount = 1;
		VkPipeline pipeline = nullptr;
		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, createInfoCount, &graphics_pipeline_create_info, VK_NULL_HANDLE, &pipeline);
		
		PipelineData data;
		data.pipeline = pipeline;
		auto ret = PipelineIndex++;
		vPipeline[ret] = data;
		return ret;
	}
	
	uint32_t CreateRenderPass(
		std::vector<uint32_t> &color_attachments,
		std::vector<uint32_t> &depth_attachments,
		uint32_t width, uint32_t height, uint32_t format_type = 0, uint32_t bind_type = 0)
	{
		std::vector<VkAttachmentDescription> attachmentDescs;
		std::vector<VkAttachmentReference>   attachmentRefs;
		uint32_t attachment_index = 0;
		for(int i = 0 ; i < color_attachments.size(); i++) {
			VkAttachmentDescription attachmentdesc = {};
			attachmentdesc.samples        = VK_SAMPLE_COUNT_1_BIT;
			attachmentdesc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentdesc.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentdesc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentdesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentdesc.format         = format_type ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_B8G8R8A8_UNORM;
			attachmentdesc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentdesc.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			attachmentDescs.push_back(attachmentdesc);
			
			VkAttachmentReference attachmentref = {};
			attachmentref.attachment = attachment_index;
			attachmentref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentRefs.push_back(attachmentref);
			attachment_index++;
		}
		for(int i = 0 ; i < depth_attachments.size(); i++) {
			VkAttachmentDescription attachmentdesc = {};
			attachmentdesc.samples        = VK_SAMPLE_COUNT_1_BIT;
			attachmentdesc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentdesc.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentdesc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentdesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentdesc.format         = VK_FORMAT_D32_SFLOAT;
			attachmentdesc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentdesc.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			attachmentDescs.push_back(attachmentdesc);

			VkAttachmentReference attachmentref = {};
			attachmentref.attachment = attachment_index;
			attachmentref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachmentRefs.push_back(attachmentref);
			attachment_index++;
		}

		VkSubpassDescription subpassdesc = {};
		subpassdesc.pipelineBindPoint = bind_type ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassdesc.colorAttachmentCount = attachmentRefs.size() - 1; //-1 for depth
		subpassdesc.pColorAttachments = attachmentRefs.data();
		subpassdesc.pDepthStencilAttachment = attachmentRefs.data() + subpassdesc.colorAttachmentCount;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = attachmentDescs.size();
		info.pAttachments = attachmentDescs.data();
		info.subpassCount = 1;
		info.pSubpasses = &subpassdesc;

		VkRenderPass renderpass = nullptr;
		vkCreateRenderPass(device, &info, nullptr, &renderpass);
		printf("vkCreateRenderPass=%016X\n", renderpass);
		
		//create frame buffer
		VkFramebuffer framebuffer = nullptr;
		{
			VkFramebufferCreateInfo fbinfo = {};
			fbinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbinfo.renderPass = renderpass;
			std::vector<VkImageView> image_views_attach;
			for(int i = 0 ; i < color_attachments.size(); i++) {
				ImageData image_data = vImage[color_attachments[i]];
				image_views_attach.push_back(image_data.view);
			}
			for(int i = 0 ; i < depth_attachments.size(); i++) {
				ImageData image_data = vImage[depth_attachments[i]];
				image_views_attach.push_back(image_data.view);
			}
			
			fbinfo.attachmentCount = image_views_attach.size();
			fbinfo.pAttachments = image_views_attach.data();
			fbinfo.width = width;
			fbinfo.height = height;
			fbinfo.layers = 1;
			vkCreateFramebuffer(device, &fbinfo, nullptr, &framebuffer);
			printf("vkCreateFramebuffer=%016X\n", framebuffer);
		}
		
		//Create CommandBuffer
		VkCommandBuffer commandbuffer = nullptr;
		{
			VkCommandBufferAllocateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			info.commandPool = commandpool;
			info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			info.commandBufferCount = 1;
			vkAllocateCommandBuffers(device, &info, &commandbuffer);
			printf("vkAllocateCommandBuffers=%016X\n", commandbuffer);
		}
		
		//Create Fence
		VkFence fence = nullptr;
		{
			VkFenceCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			vkCreateFence(device, &info, NULL, &fence);
			printf("vkCreateFence=%016X\n", fence);
			vkResetFences(device, 1, &fence);
			printf("DO vkResetFences=%016X\n", fence);
		}
		
		RenderPassData data = {};
		data.renderpass = renderpass;
		data.framebuffer = framebuffer;
		data.commandbuffer = commandbuffer;
		data.fence = fence;
		data.color_attachments = color_attachments;
		data.depth_attachments = depth_attachments;
		data.width = width;
		data.height = height;
		data.bind_point = subpassdesc.pipelineBindPoint;
		
		uint32_t ret = RenderPassIndex++;
		vRenderPass[ret] = data;
		printf("vRenderPass %d\n", ret);
		return ret;
	}

	uint32_t CreateBuffer(size_t size, uint32_t type = 0, void *src = nullptr) { //vertex, uniform
		BufferData data;
		VkBuffer buffer = nullptr;
		VkBufferCreateInfo info = {};
		VkMemoryRequirements memreq = {};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size  = size;
		info.usage = 0; 
		info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		vkCreateBuffer(device, &info, nullptr, &buffer);
		vkGetBufferMemoryRequirements(device, buffer, &memreq);
		memreq.size  = memreq.size + (memreq.alignment - 1);
		memreq.size &= ~(memreq.alignment - 1);
		printf("memreq.size=%d\n", memreq.size);
		DeviceMemoryData memdata = vDeviceMemory[MEMORY_TYPE_HOST];
		vkBindBufferMemory(device, buffer, memdata.devicememory, memdata.offset);
		VkDeviceSize offset = memdata.offset;
		memdata.offset += memreq.size;
		if(src) {
			void *dest = nullptr;
			vkMapMemory(device, memdata.devicememory, offset, size, 0, (void **)&dest);
			if(dest) {
				printf("コピーできているはず\n");
				memcpy(dest, src, size);
				vkUnmapMemory(device, memdata.devicememory);
			} else {
				printf("MAPがぜんぜんできてない！\n");
			}
		}

		vDeviceMemory[MEMORY_TYPE_HOST] = memdata;
		
		data.buffer = buffer;
		data.info = info;
		data.memreq = memreq;
		uint32_t ret = BufferIndex++;
		vBuffer[ret] = data;
		return ret;
	}

	VkDeviceSize GetBufferSize(uint32_t handle) {
		auto ret = vBuffer.find(handle);
		if(ret != vBuffer.end()) {
			return ret->second.memreq.size;
		}
		printf("%s : Error\n", __FUNCTION__);
		return 0;
	}

	uint32_t CreateShader(const char *entrypointname, const void *shaderdata, size_t size, uint32_t type) {
		VkShaderModuleCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = size;
		info.pCode = (uint32_t *)shaderdata;
		VkShaderModule shader = nullptr;
		vkCreateShaderModule(device, &info, nullptr, &shader);
		ShaderData data;
		data.type = type;
		data.shader = shader;
		data.entrypoint = entrypointname;
		uint32_t ret = ShaderIndex++;
		vShader[ret] = data;
		return ret;
	}

	uint32_t CreateImage(uint32_t width, uint32_t height, uint32_t type, VkImage image = nullptr) {
		ImageData data = {};
		VkImageCreateInfo info = {};
		VkImageAspectFlags aspectMask = 0;
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = NULL;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = 1;
		info.format = VK_FORMAT_B8G8R8A8_UNORM;
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		data.isbackbuffer = false;
		if(type & ImageData::TYPE_BACKBUFFER) {
			data.isbackbuffer = true;
			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		if(type & ImageData::TYPE_TEXTURE) {
			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		if(type & ImageData::TYPE_RENDERTARGET) {
			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		if(type & ImageData::TYPE_DEPTH) {
			info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			info.format = VK_FORMAT_D32_SFLOAT;
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		
		info.mipLevels             = 1;
		info.arrayLayers           = 1;
		info.samples               = VK_SAMPLE_COUNT_1_BIT;
		info.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;
		info.queueFamilyIndexCount = 0;
		info.pQueueFamilyIndices   = NULL;
		info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
		info.flags                 = 0;

		//Create Image
		VkMemoryRequirements memreq = {};
		if(!image) {
			VkImage temp_image = nullptr;
			vkCreateImage(device, &info, NULL, &temp_image);
			vkGetImageMemoryRequirements(device, temp_image, &memreq);
			memreq.size  = memreq.size + (memreq.alignment - 1);
			memreq.size &= ~(memreq.alignment - 1);
		
			DeviceMemoryData data = vDeviceMemory[MEMORY_TYPE_LOCAL];
			vkBindImageMemory(device, temp_image, data.devicememory, data.offset);
			data.offset += memreq.size;
			vDeviceMemory[MEMORY_TYPE_LOCAL] = data;
			image = temp_image;
		}
		
		//Create View
		VkImageView view = nullptr;
		VkDescriptorImageInfo desc_image_info = {};
		{
			VkImageView temp_view = nullptr;
			VkImageViewCreateInfo view_info = {};
			view_info.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.pNext        = NULL;
			view_info.format       = info.format;
			view_info.components.r = VK_COMPONENT_SWIZZLE_R;
			view_info.components.g = VK_COMPONENT_SWIZZLE_G;
			view_info.components.b = VK_COMPONENT_SWIZZLE_B;
			view_info.components.a = VK_COMPONENT_SWIZZLE_A;
			view_info.subresourceRange.aspectMask = aspectMask; //VK_IMAGE_ASPECT_COLOR_BIT; //VK_IMAGE_ASPECT_DEPTH_BIT
			view_info.subresourceRange.baseMipLevel   = 0;
			view_info.subresourceRange.levelCount     = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount     = 1;
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.flags = 0;
			view_info.image = image;
		    vkCreateImageView(device, &view_info, NULL, &temp_view);
			view = temp_view;
			
			desc_image_info.sampler = nullptr;
			desc_image_info.imageView = view;
			desc_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		data.image = image;
		data.view = view;
		data.info = info;
		data.desc_image_info = desc_image_info;
		data.memreq = memreq;
		uint32_t ret = ImageIndex++;
		vImage[ret] = data;
		return ret;
	}

	uint32_t BeginPass(uint32_t pass, float r, float g, float b, float a) {
		auto passdata = vRenderPass[pass];
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		vkBeginCommandBuffer(passdata.commandbuffer, &begin_info);

		VkRenderPassBeginInfo renderpass_info = {};
		renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpass_info.renderArea.offset.x = 0;
		renderpass_info.renderArea.offset.y = 0;
		renderpass_info.renderArea.extent.width = passdata.width;
		renderpass_info.renderArea.extent.height = passdata.height;
		std::vector<VkClearValue> clearvalues;
		for(int i = 0; i < passdata.color_attachments.size(); i++) {
			VkClearValue temp;
			temp.color = { { r, g, b, a} };
			clearvalues.push_back(temp);
		}
		for(int i = 0; i < passdata.depth_attachments.size(); i++) {
			VkClearValue temp;
			temp.depthStencil = { 1.0f, 0 };
			clearvalues.push_back(temp);
		}
		renderpass_info.clearValueCount = clearvalues.size();
		renderpass_info.pClearValues = clearvalues.data();
		renderpass_info.renderPass = passdata.renderpass;
		renderpass_info.framebuffer = passdata.framebuffer;
		vkCmdBeginRenderPass(passdata.commandbuffer, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
		return 0;
	}
	
	uint32_t EndPass(uint32_t pass) {
		auto passdata = vRenderPass[pass];
		vkCmdEndRenderPass(passdata.commandbuffer);
		vkEndCommandBuffer(passdata.commandbuffer);
		return 0;
	}

	void SetPipeline(uint32_t pass, uint32_t pipeline) {
		auto passdata = vRenderPass[pass];
		auto pipelinedata = vPipeline[pipeline];
		vkCmdBindPipeline(passdata.commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinedata.pipeline);
	}

	void SetDescriptorSets(uint32_t pass) {
		auto passdata = vRenderPass[pass];
		VkDescriptorSet descriptorSet = descSet;
		vkCmdBindDescriptorSets(passdata.commandbuffer, passdata.bind_point, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	}

	uint32_t Draw(uint32_t pass, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
		auto passdata = vRenderPass[pass];
		vkCmdDraw(passdata.commandbuffer, vertexCount, instanceCount, firstVertex, firstInstance);
		return 0;
	}
	
	uint32_t PushUniform(uint32_t pass, uint32_t uniform) {
		return 0;
	}

	uint32_t PushImage(uint32_t pass, uint32_t index, uint32_t image) {
		return 0;
	}

	uint32_t SetVertexBuffer(uint32_t pass, uint32_t vertex) {
		auto passdata = vRenderPass[pass];
		auto bufferdata = vBuffer[vertex];
		VkBuffer temp = bufferdata.buffer;
		VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(passdata.commandbuffer, 0, 1, &temp, offsets);
		return 0;
	}

	uint32_t SetViewport(uint32_t pass, uint32_t width, uint32_t height) {
		auto passdata = vRenderPass[pass];
		VkViewport viewport = {};
		viewport.height   = (float)height;
		viewport.width    = (float)width;
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;

		VkRect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		vkCmdSetViewport(passdata.commandbuffer, 0, 1, &viewport);
		vkCmdSetScissor(passdata.commandbuffer, 0, 1, &scissor);
		return 0;
	}

	uint32_t UpdateDescripterSets() {
		std::vector<VkWriteDescriptorSet> writeDescriptorSets;
		std::vector<VkDescriptorBufferInfo> desc_buffer_infos(64);
		std::vector<VkDescriptorImageInfo> desc_image_infos(64);
		uint32_t binding_index = 0;
		{
			size_t uniform_size = 32768;
			uint32_t handle = CreateBuffer(32768);
			VkDescriptorBufferInfo *descriptorbufferinfo = &desc_buffer_infos[binding_index];
			descriptorbufferinfo->buffer = vBuffer[handle].buffer;
			descriptorbufferinfo->offset = 0;
			descriptorbufferinfo->range  = uniform_size;
			VkWriteDescriptorSet writedescriptorset = {};
			writedescriptorset.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writedescriptorset.dstSet = descSet;
			writedescriptorset.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writedescriptorset.dstBinding = binding_index;
			writedescriptorset.descriptorCount = 1;
			writedescriptorset.pBufferInfo = descriptorbufferinfo;
			writeDescriptorSets.push_back(writedescriptorset);
			binding_index++;
		}

		//s0
		auto sampler = vSampler[0];
		auto dummy_image = vImage[dummy_image_handle];
		for(int i = 0 ; i < 4; i++) {
			VkDescriptorImageInfo *descriptorimageinfo = &desc_image_infos[binding_index];
			*descriptorimageinfo = dummy_image.desc_image_info;
			descriptorimageinfo->sampler = sampler.sampler;
			VkWriteDescriptorSet writedescriptorset = {};
			writedescriptorset.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writedescriptorset.dstSet = descSet;
			writedescriptorset.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writedescriptorset.dstBinding = binding_index;
			writedescriptorset.descriptorCount = 1;
			writedescriptorset.pImageInfo = descriptorimageinfo;

			writeDescriptorSets.push_back(writedescriptorset);
			binding_index++;
		}
		vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		return 0;

	}

	uint32_t SubmitPass(uint32_t pass) {
		auto passdata = vRenderPass[pass];
		VkSubmitInfo submit_info = {};
		VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount   = 1;
		submit_info.pWaitDstStageMask    = &wait_mask;
		submit_info.waitSemaphoreCount   = 0;
		submit_info.signalSemaphoreCount = 0;
		submit_info.pWaitSemaphores      = 0;
		submit_info.pSignalSemaphores    = 0;
		submit_info.pCommandBuffers      = &passdata.commandbuffer;
		vkQueueSubmit(queue, 1, &submit_info, passdata.fence);
		return 0;
	}

	void WaitPass(uint32_t pass) {
		auto passdata = vRenderPass[pass];
		//https://chromium.googlesource.com/experimental/chromium/src/+/refs/wip/bajones/webvr_1/gpu/vulkan/vulkan_command_buffer.cc
		vkWaitForFences(device, 1, &passdata.fence, true, UINT64_MAX);
		vkResetFences(device, 1, &passdata.fence);
	}

	uint32_t NextImage(uint32_t pass) {
		uint32_t present_index = 0;
		auto passdata = vRenderPass[pass];
		auto fence = passdata.fence;
		vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, VK_NULL_HANDLE, fence, &present_index);
		vkWaitForFences(device, 1, &fence, true, UINT64_MAX);
		vkResetFences(device, 1, &fence);
		return present_index;
	}

	void Present(uint32_t index) {
		uint32_t imageIndex = index;
		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.pNext = NULL;
		info.swapchainCount = 1;
		info.pSwapchains = &swapchain;
		info.pImageIndices = &imageIndex;
		//ret.pWaitSemaphores = &waitSemaphore;
		//ret.waitSemaphoreCount = 1;
		vkQueuePresentKHR(queue, &info);
		//vkResetFences(device, 1, &swapchain_fence);
	}
	
};
