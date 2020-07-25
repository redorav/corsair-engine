#pragma once

#include "ICrRenderDevice.h"

#include <d3d12.h>

class CrRenderDeviceD3D12 final : public ICrRenderDevice
{
public:

	CrRenderDeviceD3D12();

	~CrRenderDeviceD3D12();

	virtual void InitPS() final override;

	ID3D12Device* GetD3D12Device() { return m_d3d12Device; }

private:

	virtual ICrCommandQueue* CreateCommandQueuePS(CrCommandQueueType::T type) final override;

	virtual ICrFramebuffer* CreateFramebufferPS(const CrFramebufferCreateParams& params) final override;

	virtual ICrGPUFence* CreateGPUFencePS() final override;

	virtual ICrRenderPass* CreateRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) final override;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) final override;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) final override;

	virtual ICrTexture* CreateTexturePS(const CrTextureCreateParams& params) final override;

	virtual ICrGPUStackAllocator* CreateGPUMemoryStreamPS() final override;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrGPUBufferDescriptor& params) final override;

	ID3D12Device* m_d3d12Device;


};
