#pragma once

#include "ICrRenderDevice.h"

class CrRenderDeviceD3D12 final : public ICrRenderDevice
{
public:

	CrRenderDeviceD3D12();

	~CrRenderDeviceD3D12();

	virtual void InitPS(void* platformHandle, void* platformWindow) final override;

	// TODO Actually make PS by making virtual
	void WaitForFencePS(const CrGPUFenceD3D12* fence, uint64_t timeoutNanoseconds);

	void ResetFencePS(const CrGPUFenceD3D12* fence);

private:

	virtual ICrCommandQueue* CreateCommandQueuePS(CrCommandQueueType::T type) final override;

	virtual ICrFramebuffer* CreateFramebufferPS(const CrFramebufferCreateParams& params) final override;
	
	virtual ICrGPUFence* CreateGPUFencePS() final override;

	virtual ICrRenderPass* CreateRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) final override;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) final override;

	virtual ICrSwapchain* CreateSwapchainPS(uint32_t requestedWidth, uint32_t requestedHeight) final override;

	virtual ICrTexture* CreateTexturePS(const CrTextureCreateParams& params) final override;

	virtual ICrGPUStackAllocator* CreateGPUMemoryStreamPS() final override;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrGPUBufferCreateParams& params) final override;
};
