#pragma once

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <map>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

#define debug_printf(...)
//#define debug_printf  debug_printf


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
			debug_printf("Cant Open File -> %s\n", name);
		}
	}

	void *Buf() {
		return (void *)(&buf[0]);
	}
};


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT,
	uint64_t object,
	size_t location,
	int32_t messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* pUserData);

inline void BindDebugFunction(VkInstance instance) {
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
		debug_printf("PFN_vkCreateDebugReportCallbackEXT IS NULL\n");
	}
}

inline void CreateProcessSync(char *szFileName) {
	PROCESS_INFORMATION pi;
	STARTUPINFO si = {};
	si.cb = sizeof(si);
	if (CreateProcess(NULL, (LPTSTR)szFileName, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		while (WaitForSingleObject(pi.hProcess, 0) != WAIT_OBJECT_0) {
			Sleep(2);
		}
	}
}

inline void CompileShader(const char *compilername, const char *filename, const char *optionstr, std::vector<unsigned char> &vdata) {
	//Compile Offline
	std::string infile   = std::string(filename);
	std::string outfile  = std::string(filename) + ".spv";
	std::string compiler = std::string(compilername);
	std::string option   = std::string(optionstr);
	std::string command  = compiler + " " + option + " " + outfile + " " + infile;
	printf("%s:command=%s\n", __FUNCTION__, command.c_str());
	CreateProcessSync((char *)command.c_str());
	File file(outfile.c_str());
	vdata = file.buf;
}

struct VertexInputBindingDescription : public VkVertexInputBindingDescription {
	VertexInputBindingDescription(
		uint32_t index,
		uint32_t value = 0,
		VkVertexInputRate rate = VK_VERTEX_INPUT_RATE_VERTEX)
	{
		memset(this, 0, sizeof(*this));
		binding = index;
		stride = value;
		inputRate = rate;
	}
};

struct PipelineInputAssemblyStateCreateInfo : public VkPipelineInputAssemblyStateCreateInfo {
	PipelineInputAssemblyStateCreateInfo() {
		memset(this, 0, sizeof(*this));
		sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
};

struct PipelineVertexInputStateCreateInfo : public VkPipelineVertexInputStateCreateInfo {
	PipelineVertexInputStateCreateInfo(
		uint32_t count,
		VkVertexInputBindingDescription *vertexInputBindingDesc,
		uint32_t input_count,
		VkVertexInputAttributeDescription *vertexInputAttr)
	{
		memset(this, 0, sizeof(*this));
		sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexBindingDescriptionCount = count;
		pVertexBindingDescriptions = vertexInputBindingDesc;
		vertexAttributeDescriptionCount = input_count;
		pVertexAttributeDescriptions = vertexInputAttr;
	}
};

//Create Rasterizer State
struct PipelineRasterizationStateCreateInfo : public VkPipelineRasterizationStateCreateInfo {
	PipelineRasterizationStateCreateInfo() {
		memset(this, 0, sizeof(*this));
		sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		polygonMode = VK_POLYGON_MODE_FILL;
		cullMode    = VK_CULL_MODE_NONE; //VK_CULL_MODE_BACK_BIT;
		frontFace   = VK_FRONT_FACE_CLOCKWISE;
		lineWidth   = 1.0f;
		depthClampEnable = VK_FALSE;
	}
};

//Create ColorBlend State
struct PipelineColorBlendStateCreateInfo : public VkPipelineColorBlendStateCreateInfo {
	std::vector<VkPipelineColorBlendAttachmentState> data;
	PipelineColorBlendStateCreateInfo() {
		memset(this, 0, sizeof(*this));
		Clear();
		sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	}
	void Clear() {
		data.clear();
	}
	void Append(VkBool32 blendEnable) {
		VkPipelineColorBlendAttachmentState temp = {};
		temp.colorWriteMask = 0xF;
		temp.blendEnable    = blendEnable;
		data.push_back(temp);

		attachmentCount = data.size();
		pAttachments    = data.data();
	}
};

//Depth Stencil 
struct PipelineDepthStencilStateCreateInfo : public VkPipelineDepthStencilStateCreateInfo {
	VkBool32 GetDepthTest() {
		return depthTestEnable;
	}
	PipelineDepthStencilStateCreateInfo() {
		memset(this, 0, sizeof(*this));
		sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthTestEnable = VK_TRUE;
		depthWriteEnable = VK_TRUE;
		depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthBoundsTestEnable = VK_FALSE;
		back.failOp = VK_STENCIL_OP_KEEP;
		back.passOp = VK_STENCIL_OP_KEEP;
		back.compareOp = VK_COMPARE_OP_ALWAYS;
		stencilTestEnable = VK_FALSE;
		front = back;
	}
};

//Viewport State
struct PipelineViewportStateCreateInfo : public VkPipelineViewportStateCreateInfo {
	PipelineViewportStateCreateInfo() {
		memset(this, 0, sizeof(*this));
		sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportCount = 1;
		scissorCount  = 1;
	}
};

//Multisample State
struct PipelineMultisampleStateCreateInfo : public VkPipelineMultisampleStateCreateInfo {
	PipelineMultisampleStateCreateInfo() {
		memset(this, 0, sizeof(*this));
		sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	}
};

struct AttachmentDescription : public VkAttachmentDescription {
	AttachmentDescription(VkFormat formatValue, VkImageLayout init, VkImageLayout final) {
		memset(this, 0, sizeof(*this));
		samples        = VK_SAMPLE_COUNT_1_BIT;
		loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		format         = formatValue;
		initialLayout  = init;
		finalLayout    = final;
	}
};

struct AttachmentReference : public VkAttachmentReference {
	AttachmentReference(uint32_t indexValue, VkImageLayout layoutValue) {
		memset(this, 0, sizeof(*this));
		attachment = indexValue;
		layout = layoutValue;
	}
	void Dump() {
		debug_printf("AttachmentReference : attachment=%d\n", attachment);
		debug_printf("AttachmentReference : layout=%d\n", layout);
	}
};

struct DescriptorPoolSize : public VkDescriptorPoolSize {
	DescriptorPoolSize(VkDescriptorType typeValue, uint32_t countValue) {
		memset(this, 0, sizeof(*this));
		type = typeValue;
		descriptorCount = countValue;
	}
};


struct DescriptorPoolCreateInfo : public VkDescriptorPoolCreateInfo {
	DescriptorPoolCreateInfo(uint32_t count, VkDescriptorPoolSize *data, uint32_t maxsets) {
		memset(this, 0, sizeof(*this));
		sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolSizeCount = count;
		pPoolSizes = data;
		maxSets = maxsets;
	}
};

struct DescriptorSetLayoutBinding : public VkDescriptorSetLayoutBinding {
	DescriptorSetLayoutBinding(
		uint32_t bindingValue,
		VkDescriptorType type,
		uint32_t descriptorCountValue = 1,
		VkShaderStageFlags stage_flagsValue = VK_SHADER_STAGE_ALL_GRAPHICS)
	{
		memset(this, 0, sizeof(*this));
		binding = bindingValue;
		descriptorType = type;
		stageFlags = stage_flagsValue;
		descriptorCount = descriptorCountValue;
		debug_printf("binding=%d\n", binding);
	}
};

struct DescriptorSetLayoutCreateInfo : public VkDescriptorSetLayoutCreateInfo {
	DescriptorSetLayoutCreateInfo(uint32_t count, VkDescriptorSetLayoutBinding* data) {
		memset(this, 0, sizeof(*this));
		sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		pBindings = data;
		bindingCount = count;
	}
};

struct DescriptorSetAllocateInfo : public VkDescriptorSetAllocateInfo {
	DescriptorSetAllocateInfo(VkDescriptorPool descPool, uint32_t descCount, VkDescriptorSetLayout *pLayouts) {
		memset(this, 0, sizeof(*this));
		sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorPool = descPool;
		descriptorSetCount = descCount;
		pSetLayouts = pLayouts;
	}
};


/*
struct WriteDescriptorSet : public VkWriteDescriptorSet {
	WriteDescriptorSet(uint32_t _descriptorCount, VkDescriptorType _descriptorType, const VkDescriptorBufferInfo *_pBufferInfo) {
		memset(this, 0, sizeof(*this));
		VkWriteDescriptorSet temp = {};
		temp.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		temp.dstSet = dstSet;
		temp.descriptorType = type;
		temp.dstBinding = binding;
		temp.pBufferInfo = bufferInfo;
		temp.descriptorCount = descriptorCount;
	}
};
*/

struct DescriptorBufferInfo : VkDescriptorBufferInfo {
	DescriptorBufferInfo(VkBuffer _buffer, VkDeviceSize _offset, VkDeviceSize _range) {
		memset(this, 0, sizeof(*this));
		buffer = _buffer;
		offset = _offset;
		range  = _range;
	}
	VkWriteDescriptorSet GetWriteDescriptorSet(VkDescriptorSet descSet, uint32_t binding) {
		VkWriteDescriptorSet ret = {};
		ret.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ret.dstSet = descSet;
		ret.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ret.dstBinding = binding;
		debug_printf("%s : ret.dstBinding=%d\n", __FUNCTION__, ret.dstBinding);
		ret.descriptorCount = 1;
		ret.pBufferInfo = this;
		return ret;
	}
};

struct DescriptorImageInfo : public VkDescriptorImageInfo {
	DescriptorImageInfo(
		VkSampler _sampler, VkImageView _imageView, VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_GENERAL) {
		memset(this, 0, sizeof(*this));
		sampler = _sampler;
		imageView = _imageView;
		imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	VkWriteDescriptorSet GetWriteDescriptorSet(VkDescriptorSet descSet, uint32_t binding) {
		VkWriteDescriptorSet ret = {};
		ret.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ret.dstSet = descSet;
		ret.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		ret.dstBinding = binding;
		debug_printf("%s : ret.dstBinding=%d\n", __FUNCTION__, ret.dstBinding);
		ret.descriptorCount = 1;
		ret.pImageInfo = this;
		return ret;
	}
};



//Dynamic State
struct PipelineDynamicStateCreateInfo : public VkPipelineDynamicStateCreateInfo {
	std::vector<VkDynamicState> data;
	void Update() {
		sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		pDynamicStates = data.data();
		dynamicStateCount = data.size();
	}
	PipelineDynamicStateCreateInfo() {
		memset(this, 0, sizeof(*this));
		data.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		data.push_back(VK_DYNAMIC_STATE_SCISSOR);
		Update();
	}
};

struct SubpassDescription : public VkSubpassDescription {
	SubpassDescription(uint32_t ref_count, VkAttachmentReference *ref_color, VkAttachmentReference *ref_depth) {
		memset(this, 0, sizeof(*this));
		pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		colorAttachmentCount    = ref_count;
		pColorAttachments       = ref_color;
		pDepthStencilAttachment = ref_depth;
	}
};


struct RenderPassCreateInfo : public VkRenderPassCreateInfo {
	RenderPassCreateInfo(
		uint32_t attach_count,
		VkAttachmentDescription *attach_desc,
		uint32_t subpass_count,
		VkSubpassDescription *subpass_desc)
	{
		memset(this, 0, sizeof(*this));
		sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		attachmentCount = attach_count;
		pAttachments = attach_desc;
		subpassCount = subpass_count;
		pSubpasses = subpass_desc;
	}
};



struct SubpassDependency : public VkSubpassDependency {
	SubpassDependency() {
		memset(this, 0, sizeof(*this));
		Default();
	}
	void Default() {
		srcSubpass = VK_SUBPASS_EXTERNAL;
		dstSubpass = 0;
		srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	}
	void Invert() {
		VkSubpassDependency temp = *this;
		dstSubpass    = srcSubpass;
		dstStageMask  = srcStageMask;
		dstAccessMask = srcAccessMask;
		srcSubpass    = temp.dstSubpass;
		srcStageMask  = temp.dstStageMask;
		srcAccessMask = temp.dstAccessMask;
	}
};

struct CommandBufferBeginInfo : public VkCommandBufferBeginInfo {
	CommandBufferBeginInfo() {
		memset(this, 0, sizeof(*this));
		sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	}
};



struct Semaphore {
	VkSemaphore data;
	VkSemaphore Get() {
		return data;
	}
	Semaphore(VkDevice device) {
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = nullptr;
		vkCreateSemaphore(device, &info, nullptr, &data);
	}
};

struct Fence {
	VkFence data;
	VkFenceCreateInfo info = {};
	VkFence Get() {
		return data;
	}
	Fence(VkDevice device) {
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(device, &info, NULL, &data);
	}
};

struct Sampler {
	VkSampler sampler;
	VkSampler Get() {
		return sampler;
	}
	Sampler(VkDevice device) {
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
	}
};


struct MemoryAllocator {
	VkDevice device = nullptr;
	VkDeviceMemory device_memory = nullptr;
	VkDeviceSize offset = 0;

	struct Tag {
		VkDeviceSize buffer_size;
		VkDeviceSize offset;
		uint32_t     type;
		bool         is_dirty;
		enum {
			Image = 0x01,
			Buffer = 0x02,
		};
	};
	std::map<void *, Tag> mapping;

	void Dump() {
		for(auto &x : mapping) {
			auto *type_str = x.second.type == Tag::Image ? "IMAGE" : "BUFFER";
			auto *is_dirty_str = x.second.is_dirty ? "DIRTY" : "NOT DIRTY";
			debug_printf("Offset = %016zX, type=%s is_dirty=%s Size=%zd\n", x.second.offset, type_str, is_dirty_str, x.second.buffer_size);
		}
	}

	MemoryAllocator(
		VkDevice d,
		VkPhysicalDeviceMemoryProperties  devicememoryprop,
		VkDeviceSize size,
		bool is_local = false)
	{
		device = d;
		VkMemoryAllocateInfo info = {};

		VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		if(is_local) {
			flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}

		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.allocationSize = size;
		for (uint32_t i = 0; i < devicememoryprop.memoryTypeCount; i++) {
			if ((devicememoryprop.memoryTypes[i].propertyFlags & flags) == flags) {
				info.memoryTypeIndex = i;
				break;
			}
		}
		vkAllocateMemory(device, &info, nullptr, &device_memory);
	}

	void Bind(VkImage image, VkDeviceSize size) {
		vkBindImageMemory(device, image, device_memory, offset);
		mapping[image] = {size, offset, Tag::Image, false};
		offset += size;
	}

	void Bind(VkBuffer buffer, VkDeviceSize size) {
		vkBindBufferMemory(device, buffer, device_memory, offset);
		mapping[buffer] = {size, offset, Tag::Buffer, false};
		offset += size;
	}

	void *Map(void *memorykey, VkDeviceSize custom_size = 0) {
		auto ret = nullptr;
		auto info = mapping.find(memorykey);
		if(info != mapping.end()) {
			VkDeviceSize mapsize = info->second.buffer_size;
			if(custom_size != 0) {
				mapsize = custom_size;
			}
			vkMapMemory(device, device_memory, info->second.offset, mapsize, 0, (void **)&ret);
		}
		return ret;
	}

	void *Update(void *memorykey) {
		auto ret = nullptr;
		auto info = mapping.find(memorykey);
		if(info != mapping.end()) {
			info->second.is_dirty = true;
			mapping[memorykey] = info->second;
		}
		return ret;
	}

	void Unmap(void *memorykey = nullptr) {
		vkUnmapMemory(device, device_memory);
	}
};

struct SwapchainKHR {
	VkSwapchainKHR data;
	std::vector<VkImage> images;
	VkFormat format;
	uint32_t imageIndex = 0;
	VkSwapchainKHR Get() {
		return data;
	}
	VkFormat GetFormat() {
		return format;
	}
	VkImage GetImage(uint32_t index) {
		return images[index];
	}

	VkPresentInfoKHR GetPresentInfoKHR(uint32_t index) {
		imageIndex = index;
		VkPresentInfoKHR ret = {};
		ret.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		ret.pNext = NULL;
		ret.swapchainCount = 1;
		ret.pSwapchains = &data;
		ret.pImageIndices = &imageIndex;
		//ret.pWaitSemaphores = &waitSemaphore;
		//ret.waitSemaphoreCount = 1;
		return ret;
	}

	SwapchainKHR(
		VkDevice device,
		VkSurfaceKHR surface,
		uint32_t width,
		uint32_t height,
		uint32_t SwapImageCount,
		VkFormat fmt = VK_FORMAT_B8G8R8A8_UNORM)
	{
		debug_printf("LINE:%08d : SwapImageCount=%d\n", __LINE__, SwapImageCount);
		VkSwapchainCreateInfoKHR info = {};
		info.sType              = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface            = surface;
		info.minImageCount      = SwapImageCount;
		info.imageFormat        = fmt;
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

		auto ret = vkCreateSwapchainKHR(device, &info, nullptr, &data);
		debug_printf("LINE:%08d : vkCreateSwapchainKHR Done ret=%d\n", __LINE__, ret);
		
		uint32_t count = 0;
		vkGetSwapchainImagesKHR(device, data, &count, nullptr);
		images.resize(count);
		vkGetSwapchainImagesKHR(device, data, &count, images.data());
		
		format = fmt;
		for(int i = 0; i < count; i++) {
			debug_printf("%s : this=%016zX, Create %016zX\n", __FUNCTION__, this, GetImage(i));
		}
	}
};

struct Image {
	VkImage data;
	VkImageCreateInfo info = {};
	VkMemoryRequirements memreq;
	bool need_release = true;

	VkImage Get() {
		return data;
	}

	VkDeviceSize GetSize() {
		return memreq.size;
	}


	VkFormat GetFormat() {
		return info.format;
	}
	uint32_t GetWidth() {
		return info.extent.width;
	}
	uint32_t GetHeight() {
		return info.extent.height;
	}

	Image(
		VkImage  image,
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
	{
		info.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext                 = NULL;
		info.tiling                = VK_IMAGE_TILING_OPTIMAL;
		info.imageType             = VK_IMAGE_TYPE_2D;
		info.extent.width          = width;
		info.extent.height         = height;
		info.format                = format;
		info.extent.depth          = 1;
		info.mipLevels             = 1;
		info.arrayLayers           = 1;
		info.samples               = VK_SAMPLE_COUNT_1_BIT;
		info.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;
		info.queueFamilyIndexCount = 0;
		info.pQueueFamilyIndices   = NULL;
		info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
		info.flags                 = 0;
		info.usage                 = usageFlags;
		need_release = false;
		data = image;
	}

	Image(
		VkDevice device,
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageUsageFlags usageFlags)
	{
		//VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		info.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext                 = NULL;
		info.tiling                = VK_IMAGE_TILING_OPTIMAL;
		info.imageType             = VK_IMAGE_TYPE_2D;
		info.extent.width          = width;
		info.extent.height         = height;
		info.format                = format;
		info.extent.depth          = 1;
		info.mipLevels             = 1;
		info.arrayLayers           = 1;
		info.samples               = VK_SAMPLE_COUNT_1_BIT;
		info.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;
		info.queueFamilyIndexCount = 0;
		info.pQueueFamilyIndices   = NULL;
		info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
		info.flags                 = 0;
		info.usage                 = usageFlags;
		vkCreateImage(device, &info, NULL, &data);
		vkGetImageMemoryRequirements(device, data, &memreq);
		memreq.size  = memreq.size + (memreq.alignment - 1);
		memreq.size &= ~(memreq.alignment - 1);
		debug_printf("%s : this=%016zX, Create %016zX\n", __FUNCTION__, this, Get());
	}
};

struct ImageView {
	VkImageView data;
	VkDescriptorImageInfo image_info;
	
	VkImageView Get() {
		return data;
	}
	VkWriteDescriptorSet GetWriteDescriptorSet(VkDescriptorSet descSet, VkSampler sampler, uint32_t binding) {
		image_info.sampler = sampler;
		image_info.imageView = Get();
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet ret = {};
		ret.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ret.dstSet = descSet;
		ret.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		ret.dstBinding = binding;
		debug_printf("%s : ret.dstBinding=%d\n", __FUNCTION__, ret.dstBinding);
		ret.descriptorCount = 1;
		ret.pImageInfo = &image_info;
		return ret;
	}
	
	ImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectMask) {
		VkImageViewCreateInfo info = {};
		info.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.pNext        = NULL;
		info.format       = format; //VK_FORMAT_D16_UNORM
		info.components.r = VK_COMPONENT_SWIZZLE_R;
		info.components.g = VK_COMPONENT_SWIZZLE_G;
		info.components.b = VK_COMPONENT_SWIZZLE_B;
		info.components.a = VK_COMPONENT_SWIZZLE_A;
		info.subresourceRange.aspectMask = aspectMask; //VK_IMAGE_ASPECT_COLOR_BIT; //VK_IMAGE_ASPECT_DEPTH_BIT
		info.subresourceRange.baseMipLevel   = 0;
		info.subresourceRange.levelCount     = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount     = 1;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.flags = 0;
		info.image = image;
	    vkCreateImageView(device, &info, NULL, &data);
		debug_printf("%s : this=%016zX, Create %016zX\n", __FUNCTION__, this, Get());
	}
};


struct Buffer {
	VkBuffer data = nullptr;
	VkBufferCreateInfo info = {};
	VkMemoryRequirements memreq = {};
	VkBuffer Get() {
		return data;
	}
	VkDeviceSize GetSize() {
		return memreq.size;
	}
	VkDeviceSize GetUpdateSize() {
		return info.size;
	}
	
	Buffer(VkDevice device, VkDeviceSize size) {
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size  = size;
		info.usage = 0; 
		info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		vkCreateBuffer(device, &info, nullptr, &data);
		vkGetBufferMemoryRequirements(device, data, &memreq);
		memreq.size  = memreq.size + (memreq.alignment - 1);
		memreq.size &= ~(memreq.alignment - 1);
		debug_printf("%s : this=%016zX, Create %016zX, size=%d, memreq.size=%d\n", __FUNCTION__, this, Get(), size, memreq.size);
	}
	
};

struct DescriptorPool {
	VkDescriptorPool     data = nullptr;
	VkDescriptorPoolCreateInfo info = {};
	VkDescriptorPool Get() {
		return data;
	}
	
	//https://github.com/EpicGames/UnrealEngine/blob/1d2c1e48bf49836a4fee1465be87ab3f27d5ae3a/Engine/Source/Runtime/VulkanRHI/Private/VulkanPendingState.cpp#L20
	DescriptorPool(VkDevice device, uint32_t countValue, uint32_t maxsets) {
		std::vector<DescriptorPoolSize> vPoolSize;

		//https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkDescriptorType.html
		vPoolSize.push_back(DescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, countValue));
		vPoolSize.push_back(DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, countValue));
		vPoolSize.push_back(DescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, countValue));
		vPoolSize.push_back(DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, countValue));
		vPoolSize.push_back(DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, countValue));
		vPoolSize.push_back(DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, countValue));
		vPoolSize.push_back(DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, countValue));
		vPoolSize.push_back(DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, countValue));
		vPoolSize.push_back(DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, countValue));
		vPoolSize.push_back(DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, countValue));
		vPoolSize.push_back(DescriptorPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, countValue));

		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.poolSizeCount = vPoolSize.size();
		info.pPoolSizes = vPoolSize.data();
		info.maxSets = maxsets;

		vkCreateDescriptorPool(device, &info, nullptr, &data);
		debug_printf("%s : this=%016zX, Create %016zX\n", __FUNCTION__, this, Get());
	}
};

struct DescriptorSetLayout {
	VkDescriptorSetLayout data = nullptr;
	VkDescriptorSetLayoutCreateInfo info = {};
	const VkDescriptorSetLayout Get() {
		return data;
	}
	VkDescriptorSetLayout *GetAddress() {
		return &data;
	}
	
	DescriptorSetLayout(VkDevice device, uint32_t binding_count, VkDescriptorSetLayoutBinding *binding_data) {
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = binding_count;
		info.pBindings = binding_data;
		vkCreateDescriptorSetLayout(device, &info, nullptr, &data);
		debug_printf("%s : this=%016zX, Create %016zX\n", __FUNCTION__, this, Get());
	}
};

struct DescriptorSet {
	VkDescriptorSet data = nullptr;
	VkDescriptorSetAllocateInfo info = {};
	VkDescriptorSet Get() {
		return data;
	}
	DescriptorSet(VkDevice device, VkDescriptorPool descPool, uint32_t descCount, VkDescriptorSetLayout *pLayouts) {
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		info.descriptorPool = descPool;
		info.descriptorSetCount = descCount;
		info.pSetLayouts = pLayouts;
		vkAllocateDescriptorSets(device, &info, &data);
		debug_printf("%s : this=%016zX, Create %016zX\n", __FUNCTION__, this, Get());
	}
};

struct RenderPass {
	VkRenderPass data = nullptr;
	VkRenderPassCreateInfo info = {};
	std::vector<VkClearValue> clearValues;

	uint32_t GetClearValueCount() {
		return clearValues.size();
	}

	VkClearValue *GetClearValues() {
		return clearValues.data();
	}

	uint32_t GetAttachmentCount() {
		return info.attachmentCount;
	}

	VkRenderPass Get() {
		return data;
	}

	VkRenderPassBeginInfo GetRenderPassBeginInfo(VkFramebuffer framebuffer, uint32_t width, uint32_t height) {
		VkRenderPassBeginInfo ret = {};
		ret.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		ret.renderArea.offset.x = 0;
		ret.renderArea.offset.y = 0;
		ret.renderArea.extent.width = width;
		ret.renderArea.extent.height = height;
		ret.clearValueCount = GetClearValueCount();
		ret.pClearValues = GetClearValues();
		ret.renderPass = data;
		ret.framebuffer = framebuffer;
		return ret;
	}

	RenderPass(VkDevice device, uint32_t color_num, VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM) {
		std::vector<VkAttachmentDescription> attachmentDescs;
		std::vector<VkAttachmentReference>   attachmentRef;
		uint32_t index = 0;
		for(int i = 0 ; i < color_num; i++) {
			attachmentDescs.push_back(AttachmentDescription(color_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR));
			attachmentRef.push_back(AttachmentReference(index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
			index++;
		}
		AttachmentReference  depth_attach_ref(index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		attachmentDescs.push_back(AttachmentDescription(VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL));
		
		for(int i = 0; i < attachmentRef.size(); i++) {
			VkClearValue temp;
			temp.color = { { 1.0f, 1.0f, 1.0f, 1.0f } };
			clearValues.push_back(temp);
		}
		{
			VkClearValue temp;
			temp.depthStencil = { 1.0f, 0 };
			clearValues.push_back(temp);
		}

		SubpassDescription subpassDesc(attachmentRef.size(), attachmentRef.data(), &depth_attach_ref);
		info = RenderPassCreateInfo(attachmentDescs.size(), attachmentDescs.data(), 1, &subpassDesc);
		vkCreateRenderPass(device, &info, nullptr, &data);
		debug_printf("%s : this=%016zX, Create %016zX\n", __FUNCTION__, this, Get());
	}
};

struct FrameBuffer {
	VkFramebuffer data = nullptr;
	VkFramebufferCreateInfo info = {};
	VkFramebuffer Get() {
		return data;
	}
	uint32_t GetWidth() {
		return info.width;
	}

	uint32_t GetHeight() {
		return info.height;
	}

	VkRenderPass GetRenderPass() {
		return info.renderPass;
	}

	FrameBuffer(
		VkDevice device,
		VkRenderPass render_pass,
		std::vector<ImageView *> &image_views,
		uint32_t w,
		uint32_t h)
	{
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = render_pass;
		std::vector<VkImageView> image_views_attach;
		for(int i = 0 ; i < image_views.size(); i++) {
			ImageView *image = image_views[i];
			image_views_attach.push_back(image->Get());
		}
		info.attachmentCount = image_views_attach.size();
		info.pAttachments = image_views_attach.data();
		info.width = w;
		info.height = h;
		info.layers = 1;
		vkCreateFramebuffer(device, &info, nullptr, &data);
		debug_printf("%s : this=%016zX, Create %016zX\n", __FUNCTION__, this, Get());
	}
};

struct CommandPool {
	VkCommandPool data = nullptr;
	VkCommandPoolCreateInfo info = {};
	VkCommandPool Get() {
		return data;
	}

	CommandPool(VkDevice device, uint32_t queueFamilyIndex) {
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.pNext = NULL;
		info.queueFamilyIndex = queueFamilyIndex;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		vkCreateCommandPool(device, &info, NULL, &data);
	}
};

struct CommandBuffer {
	VkDevice device = nullptr;
	VkQueue  queue = nullptr;
	VkCommandBuffer data = nullptr;
	VkCommandBufferAllocateInfo info = {};
	Fence *submit_fence = nullptr;
	bool need_reset = false;

	VkPipelineBindPoint bindpoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkCommandBuffer Get() {
		return data;
	}

	VkFence GetFence() {
		return submit_fence->Get();
	}

	VkFence ResetFence() {
		VkFence fence = submit_fence->Get();
		vkResetFences(device, 1, &fence);
		return submit_fence->Get();
	}

	CommandBuffer(VkDevice d, VkQueue q, VkCommandPool commandPool) {
		device = d;
		queue = q;
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.commandPool = commandPool;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandBufferCount = 1;
		vkAllocateCommandBuffers(device, &info, &data);

		submit_fence = new Fence(device);

	}


	void CopyBufferToImage(VkBuffer buffer, VkImage image) {
	}

	void Reset() {
		need_reset = true;
	}
	
	void Begin() {
		CommandBufferBeginInfo begin_info;
		vkBeginCommandBuffer(data, &begin_info);
	}
	
	void End() {
		vkEndCommandBuffer(data);
	}
	void BeginPass(VkRenderPassBeginInfo *render_pass_info, uint32_t width, uint32_t height) {
		vkCmdBeginRenderPass(data, render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void EndPass() {
		vkCmdEndRenderPass(data);
	}

	void SetViewport(uint32_t width, uint32_t height) {
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

		vkCmdSetViewport(data, 0, 1, &viewport);
		vkCmdSetScissor(data, 0, 1, &scissor);
	}

	void SetDescriptorSets(VkPipelineLayout pipelineLayout, VkDescriptorSet desc) {
		VkDescriptorSet descriptorSet = desc;
		vkCmdBindDescriptorSets(data, bindpoint, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	}

	void SetPipeline(VkPipeline pipeline) {
		vkCmdBindPipeline(data, bindpoint, pipeline);
	}
	void SetVertexBuffer(VkBuffer buffer) {
		VkBuffer temp = buffer;
		VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(data, 0, 1, &temp, offsets);
	}

	void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
		vkCmdDraw(data, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void Submit(uint32_t wait_sem_num = 0, VkSemaphore *wait_sem = nullptr, uint32_t signal_sem_num = 0, VkSemaphore* signal_sem = nullptr) {
		VkSubmitInfo submit_info = {};
		VkPipelineStageFlags wait_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount   = 1;
		submit_info.pWaitDstStageMask    = &wait_mask;
		submit_info.waitSemaphoreCount   = wait_sem_num;
		submit_info.signalSemaphoreCount = signal_sem_num;
		submit_info.pWaitSemaphores      = wait_sem;
		submit_info.pSignalSemaphores    = signal_sem;
		submit_info.pCommandBuffers      = &data;
		VkFence fence = submit_fence->Get();
		vkResetFences(device, 1, &fence);
		vkQueueSubmit(queue, 1, &submit_info, fence);
	}
	
	void Wait() {
		VkFence fence = submit_fence->Get();
		//https://chromium.googlesource.com/experimental/chromium/src/+/refs/wip/bajones/webvr_1/gpu/vulkan/vulkan_command_buffer.cc
		vkWaitForFences(device, 1, &fence, true, UINT64_MAX);
	}
	
	void Present(VkPresentInfoKHR *presentInfo) {
		vkQueuePresentKHR(queue, presentInfo);
	}

	void Update() {
		if(need_reset) {
			//https://chromium.googlesource.com/experimental/chromium/src/+/refs/wip/bajones/webvr_1/gpu/vulkan/vulkan_command_buffer.cc
			Wait();
			vkResetCommandBuffer(data, 0);
		}
		need_reset = false;
	}
};

struct ShaderModule {
	VkShaderModule data = nullptr;
	VkShaderModuleCreateInfo info = {};
	std::string entrypoint_name = "";
	VkShaderModule Get() {
		return data;
	}
	
	ShaderModule(VkDevice device, const uint32_t *shader_bin, const size_t shader_bin_size, const char *name = nullptr) {
		entrypoint_name = "";
		VkShaderModuleCreateInfo vertModuleInfo = {};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = shader_bin_size;
		info.pCode = (uint32_t *)shader_bin;
		vkCreateShaderModule(device, &info, nullptr, &data);
		if(name) {
			entrypoint_name = std::string(name);
		}
	}

	/*
		VK_SHADER_STAGE_VERTEX_BIT = 0x00000001,
		VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT = 0x00000002,
		VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 0x00000004,
		VK_SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
		VK_SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
		VK_SHADER_STAGE_COMPUTE_BIT = 0x00000020,
		VK_SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
		VK_SHADER_STAGE_ALL = 0x7FFFFFFF,
	*/
	VkPipelineShaderStageCreateInfo GetPipelineShaderStageCreateInfo(VkShaderStageFlagBits flag, const char *name = nullptr) {
		if(name) {
			entrypoint_name = std::string(name);
		}
		VkPipelineShaderStageCreateInfo ret = {};
		ret.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ret.stage = flag;
		ret.module = data;
		ret.pName = entrypoint_name.c_str();
		return ret;
	}
};

struct PipelineLayout {
	VkPipelineLayout data = nullptr;
	VkPipelineLayoutCreateInfo info = {};

	VkPipelineLayout Get() {
		return data;
	}

	PipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout) {
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.setLayoutCount = 1;
		info.pSetLayouts = &descriptorSetLayout;
		vkCreatePipelineLayout(device, &info, nullptr, &data);
	}
};

struct Pipeline {
	VkPipeline data = nullptr;
	VkGraphicsPipelineCreateInfo info = {};

	VkPipeline Get() {
		return data;
	}

	Pipeline(
		VkDevice device,
		std::vector<VkVertexInputAttributeDescription> &vertexInputAttr,
		VkDeviceSize inputAttrStride,
		VkPipelineLayout pipelineLayout,
		VkRenderPass     renderpass,
		uint32_t attachmentCount,
		std::vector<VkPipelineShaderStageCreateInfo> &shaderStages)
	{
		debug_printf("vertexInputAttr size = %d\n",  vertexInputAttr.size());
		VertexInputBindingDescription         vertexInputBindingDesc(0, inputAttrStride);
		PipelineVertexInputStateCreateInfo    vertexInputState(1, &vertexInputBindingDesc, vertexInputAttr.size(), vertexInputAttr.data());
		PipelineInputAssemblyStateCreateInfo  inputState;
		PipelineRasterizationStateCreateInfo  rasterizationState;
		PipelineColorBlendStateCreateInfo     colorBlendState;
		PipelineDepthStencilStateCreateInfo   depthStencilState;
		PipelineViewportStateCreateInfo       viewportState;
		PipelineMultisampleStateCreateInfo    multisampleState;
		int attachmentCountTemp = attachmentCount;

		//decrease for depth attachment.
		if(depthStencilState.GetDepthTest()) {
			attachmentCountTemp--;
		}

		for(int i = 0; i < attachmentCountTemp; i++) {
			colorBlendState.Append(VK_FALSE);
		}
		PipelineDynamicStateCreateInfo dynamicState;

		info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		info.stageCount          = shaderStages.size();
		info.pStages             = shaderStages.data();
		info.pVertexInputState   = &vertexInputState;
		info.pInputAssemblyState = &inputState;
		info.pTessellationState  = nullptr;
		info.pViewportState      = &viewportState;
		info.pRasterizationState = &rasterizationState;
		info.pMultisampleState   = &multisampleState;
		info.pDepthStencilState  = &depthStencilState;
		info.pColorBlendState    = &colorBlendState;
		info.pDynamicState       = &dynamicState;
		info.layout              = pipelineLayout;
		info.renderPass          = renderpass;
		info.subpass             = 0;

		const uint32_t createInfoCount = 1;
		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, createInfoCount, &info, VK_NULL_HANDLE, &data);
	}
};

struct Instance {
	VkInstance                       instance;
	VkSurfaceKHR                     surface;
	VkApplicationInfo                appInfo;
	std::vector<VkPhysicalDevice>    adapters;
	VkInstanceCreateInfo             instanceInfo;
	VkPhysicalDevice                 phydevice;
	VkWin32SurfaceCreateInfoKHR      surfaceinfo;
	VkPhysicalDeviceFeatures         devicefeatures;
	VkPhysicalDeviceProperties       deviceprop;
	VkPhysicalDeviceMemoryProperties devicememoryprop;
	uint32_t queueFamilyIndex = 0xffffffff;
	VkDevice device = nullptr;
	VkQueue  queue = nullptr;
	uint32_t swapchainCount = 0;
	uint32_t adapterCount = 0;
	
	uint32_t GetPhysicalDeviceCount() {
		return adapters.size();
	}
	uint32_t GetQueueFamilyIndex() {
		return queueFamilyIndex;
	}
	VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties() {
		return devicememoryprop;
	}
	VkInstance GetInstance() { return instance; }
	VkDevice GetDevice() { return device; }
	VkQueue GetQueue() { return queue; }
	VkPhysicalDevice GetPhysicalDevice() { return phydevice; }
	VkSurfaceKHR GetSurfaceKHR() { return surface; };

	Instance(const char *appname, HWND hWnd, HINSTANCE hInst, uint32_t Width, uint32_t Height, uint32_t usageBit = VK_QUEUE_GRAPHICS_BIT) {
		memset(this, 0, sizeof(*this));

		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pApplicationName   = appname;
		appInfo.apiVersion = VK_API_VERSION_1_0;

		static const char *layers[] = { "VK_LAYER_LUNARG_standard_validation" };
		static const char *extensions[] = { "VK_KHR_swapchain" };

		static const char* extensions_info[] = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		};

		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledExtensionCount = std::size(extensions_info);
		instanceInfo.ppEnabledExtensionNames = extensions_info;
		instanceInfo.enabledLayerCount = std::size(layers);
		instanceInfo.ppEnabledLayerNames = layers;

		vkCreateInstance(&instanceInfo, nullptr, &instance);
		BindDebugFunction(GetInstance());

		//Create PhyDevice
		vkEnumeratePhysicalDevices(instance, &adapterCount, NULL);
		adapters.resize(adapterCount);
		vkEnumeratePhysicalDevices(instance, &adapterCount, adapters.data());

		//default
		phydevice = adapters[0];

		//Create Surface
		surfaceinfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceinfo.hinstance = hInst;
		surfaceinfo.hwnd      = hWnd;
		vkCreateWin32SurfaceKHR(instance, &surfaceinfo, NULL, &surface);
		debug_printf("LINE:%08d : vkCreateWin32SurfaceKHR Done\n", __LINE__);

		uint32_t surfaceFormatCount = 0;
		uint32_t presentModeCount = 0;

		vkGetPhysicalDeviceSurfaceFormatsKHR(phydevice, surface, &surfaceFormatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> vSurfaceFormatKHR(surfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(phydevice, surface, &surfaceFormatCount, vSurfaceFormatKHR.data());
		debug_printf("LINE:%08d : vkGetPhysicalDeviceSurfaceFormatsKHR done vSurfaceFormatKHR.size()=%d\n", __LINE__, vSurfaceFormatKHR.size());
		for(size_t i = 0; i < vSurfaceFormatKHR.size(); i++) {
			VkSurfaceFormatKHR fmt = vSurfaceFormatKHR[i];
			debug_printf("INFO : VkSurfaceFormatKHR : %d %d\n", fmt.format, fmt.colorSpace);
		}
	    vkGetPhysicalDeviceSurfacePresentModesKHR(phydevice, surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> vPresentModeKHR(presentModeCount);
	    vkGetPhysicalDeviceSurfacePresentModesKHR(phydevice, surface, &presentModeCount, vPresentModeKHR.data());
		debug_printf("LINE:%08d : vkGetPhysicalDeviceSurfacePresentModesKHR Done\n", __LINE__);

		vkGetPhysicalDeviceMemoryProperties(phydevice, &devicememoryprop);

		//プロパティ
		uint32_t propertyCount    = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(phydevice, &propertyCount, nullptr);
		std::vector<VkQueueFamilyProperties> properties(propertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(phydevice, &propertyCount, &properties[0]);
		for (uint32_t i = 0; i < propertyCount; i++) {
			if ((properties[i].queueFlags & usageBit) != 0) {
				queueFamilyIndex = i;
				break;
			}
		}

		//Create Device
		VkDeviceQueueCreateInfo  queueInfo = {};
		static float qPriorities[]    = { 0.0f };
		queueInfo.sType                 = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueCount            = 1;
		queueInfo.queueFamilyIndex      = queueFamilyIndex;
		queueInfo.pQueuePriorities      = qPriorities;

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

		//別にみるわけではないようなんだが
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(phydevice, 0, surface, &presentSupport);
		debug_printf("LINE:%08d : vkGetPhysicalDeviceSurfaceSupportKHR Done\n", __LINE__);
		
		VkSurfaceCapabilitiesKHR capabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phydevice, surface, &capabilities);
	}
};

