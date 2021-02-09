#pragma once

#include "Rendering/ICrRenderDevice.h"

#include <d3d12.h>

class CrRenderDeviceD3D12 final : public ICrRenderDevice
{
public:

	CrRenderDeviceD3D12(const ICrRenderSystem* renderSystem);

	~CrRenderDeviceD3D12();

	ID3D12Device* GetD3D12Device() { return m_d3d12Device; }

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

	ID3D12Device* m_d3d12Device;
};
