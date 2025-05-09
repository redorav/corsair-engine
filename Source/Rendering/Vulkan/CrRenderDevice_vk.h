#pragma once

#include "Rendering/ICrRenderDevice.h"

#include "Core/CrCoreForwardDeclarations.h"

#include "crstl/string.h"

class CrTextureVulkan;

class CrRenderDeviceVulkan final : public ICrRenderDevice
{
public:

	CrRenderDeviceVulkan(ICrRenderSystem* renderSystem, const CrRenderDeviceDescriptor& descriptor);

	~CrRenderDeviceVulkan();

	VkInstance GetVkInstance() const { return m_vkInstance; }

	VkDevice GetVkDevice() const { return m_vkDevice; }

	VkPhysicalDevice GetVkPhysicalDevice() const { return m_vkPhysicalDevice; }

	VkPipelineCache GetVkPipelineCache() const { return m_vkPipelineCache; }

	VkPhysicalDeviceProperties GetVkPhysicalDeviceProperties() const { return m_vkPhysicalDeviceProperties2.properties; }

	VmaAllocator GetVmaAllocator() const { return m_vmaAllocator; }

	uint32_t GetVkMemoryType(uint32_t typeBits, VkFlags properties) const;

	// In Vulkan, we create the queues up-front with the device so we reserve previously created queue indices
	uint32_t ReserveVkQueueIndex();

	uint32_t GetVkQueueMaxCount() const;

	uint32_t GetVkQueueFamilyIndex() const;

	VkQueue GetVkGraphicsQueue() const;

	VkCommandBuffer GetVkSwapchainCommandBuffer() const;

	VkCommandPool GetVkGraphicsCommandPool() const;

	void TransitionVkTextureToInitialLayout(const CrTextureVulkan* vulkanTexture, const cr3d::TextureState& textureState);

	void SetVkObjectName(uint64_t vkObject, VkObjectType objectType, const char* name) const;

private:

	//------------------
	// Resource Creation
	//------------------

	virtual ICrCommandBuffer* CreateCommandBufferPS(const CrCommandBufferDescriptor& descriptor) override;

	virtual ICrGPUFence* CreateGPUFencePS(bool signaled) override;

	virtual ICrGPUSemaphore* CreateGPUSemaphorePS() override;

	virtual ICrGraphicsShader* CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) override;

	virtual ICrComputeShader* CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor) override;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& descriptor) override;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) override;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) override;

	virtual ICrTexture* CreateTexturePS(const CrTextureDescriptor& descriptor) override;

	virtual ICrGraphicsPipeline* CreateGraphicsPipelinePS(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor) override;

	virtual ICrComputePipeline* CreateComputePipelinePS(const CrComputeShaderHandle& computeShader) override;

	virtual ICrGPUQueryPool* CreateGPUQueryPoolPS(const CrGPUQueryPoolDescriptor& queryPoolDescriptor) override;

	//--------------------
	// GPU Synchronization
	//--------------------

	virtual cr3d::GPUFenceResult WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) override;

	virtual cr3d::GPUFenceResult GetFenceStatusPS(const ICrGPUFence* fence) const override;

	virtual void SignalFencePS(CrCommandQueueType::T queueType, const ICrGPUFence* signalFence) override;

	virtual void ResetFencePS(const ICrGPUFence* fence) override;

	virtual void WaitIdlePS() override;

	//--------------------
	// Download and Upload
	//--------------------

	virtual uint8_t* BeginTextureUploadPS(const ICrTexture* texture) override;

	virtual void EndTextureUploadPS(const ICrTexture* texture) override;

	virtual uint8_t* BeginBufferUploadPS(const ICrHardwareGPUBuffer* destinationBuffer) override;

	virtual void EndBufferUploadPS(const ICrHardwareGPUBuffer* destinationBuffer) override;

	virtual CrHardwareGPUBufferHandle DownloadBufferPS(const ICrHardwareGPUBuffer* sourceBuffer) override;

	virtual void SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence) override;

	void RetrieveQueueFamilies();

	VkResult SelectPhysicalDevice();

	VkResult CreateLogicalDevice();

	// Vulkan-specific support query functions

	bool IsVkDeviceExtensionSupported(const crstl::string& extension);

	// Came from RenderSystem. This is the instance this device was created from
	VkInstance m_vkInstance;

	// https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#devsandqueues
	VkDevice m_vkDevice;					// Logical device
	
	VkPhysicalDevice m_vkPhysicalDevice;	// Physical device

	VkPipelineCache m_vkPipelineCache; // Centralized pipeline cache

	crstl::open_hashset<crstl::string> m_supportedDeviceExtensions;

	// TODO Make this platform-independent
	crstl::vector<VkFormat> m_supportedRenderTargetFormats;
	crstl::vector<VkFormat> m_supportedTextureFormats;
	crstl::vector<VkFormat> m_supportedDepthStencilFormats;
	crstl::vector<VkFormat> m_supportedVertexBufferFormats;

	// Stores all available memory (type) properties for the physical device

	VkPhysicalDeviceMemoryProperties m_vkPhysicalDeviceMemoryProperties;

	// Stores the different hardware limits reported by this device, e.g. maximum buffer or texture sizes, queue priorities, etc

	VkPhysicalDeviceVulkan11Properties m_vk11PhysicalDeviceProperties;

	VkPhysicalDeviceVulkan12Properties m_vk12PhysicalDeviceProperties;

	VkPhysicalDeviceVulkan13Properties m_vk13PhysicalDeviceProperties;

	VkPhysicalDeviceProperties2 m_vkPhysicalDeviceProperties2;

	// Stores device features such as supported compression, geometry shaders, etc

	VkPhysicalDeviceVulkan11Features m_vk11DeviceSupportedFeatures;

	VkPhysicalDeviceVulkan12Features m_vk12DeviceSupportedFeatures;

	VkPhysicalDeviceVulkan13Features m_vk13DeviceSupportedFeatures;

	VkPhysicalDeviceFeatures2 m_vkDeviceSupportedFeatures2;

	uint32_t m_vkVersion = 0;

	VmaAllocator m_vmaAllocator;

	// Queues
	uint32_t m_maxCommandQueues = 0;
	uint32_t m_commandQueueFamilyIndex = 0; // Index of the queue out of the available ones for our hardware
	uint32_t m_numCommandQueues;

	VkQueue m_vkGraphicsQueue;
	VkCommandPool m_vkGraphicsCommandPool;

	// Swapchain command list
	VkCommandBuffer m_vkSwapchainCommandBuffer;
};
