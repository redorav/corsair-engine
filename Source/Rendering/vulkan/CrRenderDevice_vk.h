#pragma once

#include "Rendering/ICrRenderDevice.h"

#include "Core/Containers/CrSet.h"
#include "Core/Containers/CrVector.h"
#include "Core/String/CrString.h"

#include "Core/SmartPointers/CrSharedPtr.h"

class CrRenderDeviceVulkan final : public ICrRenderDevice
{
public:

	CrRenderDeviceVulkan(const ICrRenderSystem* renderSystem);

	~CrRenderDeviceVulkan();

	const VkInstance GetVkInstance() const { return m_vkInstance; }

	const VkDevice GetVkDevice() const { return m_vkDevice; }

	const VkPhysicalDevice GetVkPhysicalDevice() const { return m_vkPhysicalDevice; }

	const VkPipelineCache GetVkPipelineCache() const { return m_vkPipelineCache; }

	uint32_t GetVkMemoryType(uint32_t typeBits, VkFlags properties) const;

	// In Vulkan, we create the queues up-front with the device so we reserve previously created queue indices
	uint32_t ReserveVkQueueIndex();

	uint32_t GetVkQueueMaxCount() const;

	uint32_t GetVkQueueFamilyIndex() const;

private:

	virtual ICrCommandQueue* CreateCommandQueuePS(CrCommandQueueType::T type) override;

	virtual ICrFramebuffer* CreateFramebufferPS(const CrFramebufferCreateParams& params) override;
	
	virtual ICrGPUFence* CreateGPUFencePS() override;

	virtual ICrGPUSemaphore* CreateGPUSemaphorePS() override;

	virtual ICrGraphicsShader* CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const override;

	virtual ICrComputeShader* CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor) const override;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& descriptor) override;

	virtual ICrRenderPass* CreateRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) override;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) override;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) override;

	virtual ICrTexture* CreateTexturePS(const CrTextureCreateParams& params) override;

	virtual ICrGraphicsPipeline* CreateGraphicsPipelinePS(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const ICrGraphicsShader* graphicsShader,
		const CrVertexDescriptor& vertexDescriptor, const CrRenderPassDescriptor& renderPassDescriptor) override;

	virtual ICrComputePipeline* CreateComputePipelinePS(const CrComputePipelineDescriptor& pipelineDescriptor, const ICrComputeShader* computeShader) override;

	virtual cr3d::GPUWaitResult WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) override;

	virtual void ResetFencePS(const ICrGPUFence* fence) override;

	virtual void WaitIdlePS() override;

	virtual bool GetIsFeatureSupported(CrRenderingFeature::T feature) const override;

	void RetrieveQueueFamilies();

	VkResult SelectPhysicalDevice();

	VkResult CreateLogicalDevice();

	// Vulkan-specific support query functions

	bool IsVkDeviceExtensionSupported(const CrString& extension);

	// TODO Make platform-independent
	bool IsDepthStencilFormatSupported(VkFormat depthFormat);

	// Came from RenderSystem. This is the instance this device was created from
	VkInstance m_vkInstance;

	// https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#devsandqueues
	VkDevice m_vkDevice;					// Logical device
	
	VkPhysicalDevice m_vkPhysicalDevice;	// Physical device
	
	VkPhysicalDeviceFeatures m_vkDeviceSupportedFeatures;

	VkPipelineCache m_vkPipelineCache; // Centralized pipeline cache

	CrSet<CrString> m_supportedDeviceExtensions;

	// TODO Make this platform-independent
	CrSet<VkFormat> m_supportedRenderTargetFormats;
	CrSet<VkFormat> m_supportedTextureFormats;
	CrSet<VkFormat> m_supportedDepthStencilFormats;
	CrSet<VkFormat> m_supportedVertexBufferFormats;

	// Stores all available memory (type) properties for the physical device
	VkPhysicalDeviceMemoryProperties m_vkPhysicalDeviceMemoryProperties;

	// Stores the different hardware limits reported by this device, e.g. maximum buffer or texture sizes, queue priorities, etc
	VkPhysicalDeviceProperties m_vkPhysicalDeviceProperties;

	// Queues
	uint32_t m_maxCommandQueues = 0;
	uint32_t m_commandQueueFamilyIndex = 0; // Index of the queue out of the available ones for our hardware
	uint32_t m_numCommandQueues;
};
