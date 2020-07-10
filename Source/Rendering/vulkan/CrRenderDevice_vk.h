#pragma once

#include "Core/Containers/CrSet.h"
#include "Core/Containers/CrVector.h"
#include "Core/String/CrString.h"

#include "ICrRenderDevice.h"

#include "CrGPUBuffer.h" // todo hack
#include "ICrShaderManager.h"
#include "ICrFramebuffer.h"
#include "ICrRenderPass.h"

#include "Core/SmartPointers/CrSharedPtr.h"

class CrGraphicsPipeline;
class CrGPUFenceVulkan;
class CrGPUSemaphoreVulkan;
class ICrCommandBuffer;

class CrRenderModel;
using CrRenderModelSharedHandle = CrSharedPtr<CrRenderModel>;

class CrRenderDeviceVulkan final : public ICrRenderDevice
{
	template<typename BufferMetadata> friend class CrConstantBuffer;

public:

	CrRenderDeviceVulkan();

	~CrRenderDeviceVulkan();

	virtual void InitPS(void* platformHandle, void* platformWindow) final override;

	virtual void PresentPS() final override;

	const VkDevice GetVkDevice() const { return m_vkDevice; }

	const VkPhysicalDevice GetVkPhysicalDevice() const { return m_vkPhysicalDevice; }

	const VkSurfaceKHR GetVkSurface() const { return m_vkSurface; }

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

	virtual ICrGPUStackAllocator* CreateGPUMemoryStreamPS() final override;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrGPUBufferCreateParams& params) final override;

	virtual ICrGPUFence* CreateGPUFencePS() final override;

	virtual ICrGPUSemaphore* CreateGPUSemaphorePS() final override;

	virtual cr3d::GPUWaitResult WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) final override;

	virtual void ResetFencePS(const ICrGPUFence* fence) final override;

	virtual void WaitIdlePS() final override;

	virtual bool GetIsFeatureSupported(CrRenderingFeature::T feature) final override;

	void RetrieveQueueFamilies();

	VkResult CreateInstance(bool enableValidationLayer);

	VkResult CreateSurface(void* platformHandle, void* platformWindow);

	VkResult CreatePhysicalDevice();

	VkResult CreateLogicalDevice(bool enableValidationLayer);

	// Vulkan-specific support query functions

	bool IsVkDeviceExtensionSupported(const CrString& extension);

	bool IsVkInstanceExtensionSupported(const CrString& extension);

	bool IsVkInstanceLayerSupported(const CrString& layer);

	// TODO Make platform-independent
	bool IsDepthStencilFormatSupported(VkFormat depthFormat);

	void RecreateSwapchain(); // TODO This should be PS

	// TODO Remove all the functions below after proper implementation
	void SetupRenderPass();
	void SetupSwapchainFramebuffer();
	void prepareVertices();
	void updateCamera();
	void preparePipelines();

	struct SimpleVertex
	{
		CrVertexElement<half, cr3d::DataFormat::RGBA16_Float> position;
		CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> normal;
		CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> tangent;
		CrVertexElement<half, cr3d::DataFormat::RG16_Float> uv;

		static CrVertexDescriptor GetVertexDescriptor()
		{
			return { decltype(position)::GetFormat(), decltype(normal)::GetFormat(), decltype(tangent)::GetFormat(), decltype(uv)::GetFormat() };
		}
	};

	// TODO Move to render system
	VkInstance m_vkInstance; // Global per-application state https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VkInstance

	// https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#devsandqueues
	VkDevice m_vkDevice;					// Logical device
	
	VkPhysicalDevice m_vkPhysicalDevice;	// Physical device
	
	VkPhysicalDeviceFeatures m_vkDeviceSupportedFeatures;

	VkSurfaceKHR m_vkSurface;

	CrVector<const char*> m_instanceLayers;

	// Supported extensions
	CrSet<CrString> m_supportedInstanceExtensions;
	CrSet<CrString> m_supportedInstanceLayers;
	CrSet<CrString> m_supportedDeviceExtensions;

	// Make this platform-independent
	CrSet<VkFormat> m_supportedRenderTargetFormats;
	CrSet<VkFormat> m_supportedTextureFormats;
	CrSet<VkFormat> m_supportedDepthStencilFormats;
	CrSet<VkFormat> m_supportedVertexBufferFormats;

	// Command buffers
	ICrCommandBuffer* m_setupCmdBuffer; // Command buffer used for setting up different resources
	CrVector<ICrCommandBuffer*> m_drawCmdBuffers; // Command buffers used for rendering

	// Stores all available memory (type) properties for the physical device
	VkPhysicalDeviceMemoryProperties m_vkPhysicalDeviceMemoryProperties;

	// Stores the different hardware limits reported by this device, e.g. maximum buffer or texture sizes, queue priorities, etc
	VkPhysicalDeviceProperties m_vkPhysicalDeviceProperties;

	// TODO Pass as param
	uint32_t m_width = 1280;
	uint32_t m_height = 720;

	// Semaphores
	// Used to coordinate operations within the graphics queue and ensure correct command ordering
	ICrGPUSemaphore* m_renderCompleteSemaphore;
	CrGPUSemaphoreVulkan* m_presentCompleteSemaphore;

	// TODO Temporary
	CrGraphicsPipeline* m_pipelineTriangleState;
	CrGraphicsPipeline* m_pipelineLineState;

	CrRenderPassSharedHandle m_renderPass;
	CrVector<CrFramebufferSharedHandle> m_frameBuffers;

	CrVertexBufferSharedHandle m_triangleVertexBuffer;
	CrIndexBufferSharedHandle m_triangleIndexBuffer;

	CrRenderModelSharedHandle m_renderModel;

	// Queues
	uint32_t m_maxCommandQueues = 0;
	uint32_t m_commandQueueFamilyIndex = 0; // Index of the queue out of the available ones for our hardware
	uint32_t m_numCommandQueues;
};
