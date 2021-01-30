#pragma once

#include "ICrRenderDevice.h"

#include <d3d12.h>

class CrRenderDeviceD3D12 final : public ICrRenderDevice
{
public:

	CrRenderDeviceD3D12();

	~CrRenderDeviceD3D12();

	virtual void InitPS() override;

	ID3D12Device* GetD3D12Device() { return m_d3d12Device; }

private:

	virtual ICrCommandQueue* CreateCommandQueuePS(CrCommandQueueType::T type) override;

	virtual ICrFramebuffer* CreateFramebufferPS(const CrFramebufferCreateParams& params) override;

	virtual ICrGPUFence* CreateGPUFencePS() override;

	virtual ICrRenderPass* CreateRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) override;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) override;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) override;

	virtual ICrTexture* CreateTexturePS(const CrTextureCreateParams& params) override;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& params) override;

	ID3D12Device* m_d3d12Device;


};
