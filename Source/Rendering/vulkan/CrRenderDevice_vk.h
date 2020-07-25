#pragma once

#include "Rendering/ICrRenderDevice.h"

#include "Core/Containers/CrSet.h"
#include "Core/Containers/CrVector.h"
#include "Core/String/CrString.h"

#include "Core/SmartPointers/CrSharedPtr.h"

class CrRenderDeviceVulkan final : public ICrRenderDevice
{
	template<typename BufferMetadata> friend class CrConstantBuffer;

public:

	CrRenderDeviceVulkan();

	~CrRenderDeviceVulkan();

	virtual void InitPS() final override;

	// TODO Move to the RenderSystem singleton
	const VkInstance GetVkInstance() const { return m_vkInstance; }

	const VkDevice GetVkDevice() const { return m_vkDevice; }

	const VkPhysicalDevice GetVkPhysicalDevice() const { return m_vkPhysicalDevice; }

	uint32_t GetVkMemoryType(uint32_t typeBits, VkFlags properties) const;

	// In Vulkan, we create the queues up-front with the device so we reserve previously created queue indices
	uint32_t ReserveVkQueueIndex();

	uint32_t GetVkQueueMaxCount() const;

	uint32_t GetVkQueueFamilyIndex() const;

private:

	virtual ICrCommandQueue* CreateCommandQueuePS(CrCommandQueueType::T type) final override;

	virtual ICrFramebuffer* CreateFramebufferPS(const CrFramebufferCreateParams& params) final override;
	
	virtual ICrRenderPass* CreateRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) final override;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) final override;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) final override;

	virtual ICrTexture* CreateTexturePS(const CrTextureCreateParams& params) final override;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrGPUBufferDescriptor& params) final override;

	virtual ICrGPUFence* CreateGPUFencePS() final override;

	virtual ICrGPUSemaphore* CreateGPUSemaphorePS() final override;

	virtual cr3d::GPUWaitResult WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) final override;

	virtual void ResetFencePS(const ICrGPUFence* fence) final override;

	virtual void WaitIdlePS() final override;

	virtual bool GetIsFeatureSupported(CrRenderingFeature::T feature) final override;

	void RetrieveQueueFamilies();

	VkResult CreateInstance(bool enableValidationLayer);

	VkResult SelectPhysicalDevice();

	VkResult CreateLogicalDevice(bool enableValidationLayer);

	// Vulkan-specific support query functions

	bool IsVkDeviceExtensionSupported(const CrString& extension);

	bool IsVkInstanceExtensionSupported(const CrString& extension);

	bool IsVkInstanceLayerSupported(const CrString& layer);

	// TODO Make platform-independent
	bool IsDepthStencilFormatSupported(VkFormat depthFormat);

	// TODO Move to RenderSystem
	// Global per-application state https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VkInstance
	VkInstance m_vkInstance;

	// https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#devsandqueues
	VkDevice m_vkDevice;					// Logical device
	
	VkPhysicalDevice m_vkPhysicalDevice;	// Physical device
	
	VkPhysicalDeviceFeatures m_vkDeviceSupportedFeatures;

	CrVector<const char*> m_instanceLayers;

	// Supported extensions
	CrSet<CrString> m_supportedInstanceExtensions;
	CrSet<CrString> m_supportedInstanceLayers;
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
