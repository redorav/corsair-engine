#pragma once

#include "Rendering/ICrRenderDevice.h"

#include "CrDescriptorHeap_d3d12.h"

#include <d3d12.h>

class CrCommandQueueD3D12;

class CrRenderDeviceD3D12 final : public ICrRenderDevice
{
public:

	CrRenderDeviceD3D12(const ICrRenderSystem* renderSystem);

	~CrRenderDeviceD3D12();

	IDXGIAdapter1* GetDXGIAdapter() const { return m_dxgiAdapter; }

	ID3D12Device* GetD3D12Device() const { return m_d3d12Device; }

	// This command queue isn't really used for anything other than to build the swapchain and do other tasks
	// that the device used to do but now requires a queue. Don't use for actual rendering
	ID3D12CommandQueue* GetD3DDirectCommandQueue() const { return m_d3d12CommandQueue; }

	crd3d::DescriptorD3D12 AllocateRTVDescriptor();

	void FreeRTVDescriptor(crd3d::DescriptorD3D12 descriptor);

	crd3d::DescriptorD3D12 AllocateDSVDescriptor();

	void FreeDSVDescriptor(crd3d::DescriptorD3D12 descriptor);

	crd3d::DescriptorD3D12 AllocateSamplerDescriptor();

	void FreeSamplerDescriptor(crd3d::DescriptorD3D12 descriptor);

private:

	virtual ICrCommandQueue* CreateCommandQueuePS(CrCommandQueueType::T type) override;

	virtual ICrGPUFence* CreateGPUFencePS() override;

	virtual ICrGPUSemaphore* CreateGPUSemaphorePS() override;

	virtual ICrGraphicsShader* CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const override;

	virtual ICrComputeShader* CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor) const override;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& descriptor) override;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) override;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) override;

	virtual ICrTexture* CreateTexturePS(const CrTextureDescriptor& descriptor) override;

	virtual ICrGraphicsPipeline* CreateGraphicsPipelinePS(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor) override;

	virtual ICrComputePipeline* CreateComputePipelinePS(const CrComputePipelineDescriptor& pipelineDescriptor, const ICrComputeShader* computeShader) override;

	virtual ICrGPUQueryPool* CreateGPUQueryPoolPS(const CrGPUQueryPoolDescriptor& queryPoolDescriptor) override;

	virtual cr3d::GPUFenceResult WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) const override;

	virtual cr3d::GPUFenceResult GetFenceStatusPS(const ICrGPUFence* fence) const override;

	virtual void ResetFencePS(const ICrGPUFence* fence) override;

	virtual void WaitIdlePS() override;

	// Heap for Render Target Views
	CrDescriptorPoolD3D12 m_d3d12RTVHeap;

	// Heap for Depth Stencil Views
	CrDescriptorPoolD3D12 m_d3d12DSVHeap;

	// Non-shader visible heap for Samplers
	CrDescriptorPoolD3D12 m_d3d12SamplerHeap;

	ID3D12CommandQueue* m_d3d12CommandQueue;

	// This is effectively the physical device
	IDXGIAdapter1* m_dxgiAdapter;

	ID3D12Device* m_d3d12Device;
};

inline crd3d::DescriptorD3D12 CrRenderDeviceD3D12::AllocateRTVDescriptor()
{
	return m_d3d12RTVHeap.Allocate();
}

inline void CrRenderDeviceD3D12::FreeRTVDescriptor(crd3d::DescriptorD3D12 descriptor)
{
	m_d3d12RTVHeap.Free(descriptor);
}

inline crd3d::DescriptorD3D12 CrRenderDeviceD3D12::AllocateDSVDescriptor()
{
	return m_d3d12DSVHeap.Allocate();
}

inline void CrRenderDeviceD3D12::FreeDSVDescriptor(crd3d::DescriptorD3D12 descriptor)
{
	m_d3d12DSVHeap.Free(descriptor);
}

inline crd3d::DescriptorD3D12 CrRenderDeviceD3D12::AllocateSamplerDescriptor()
{
	return m_d3d12SamplerHeap.Allocate();
}

inline void CrRenderDeviceD3D12::FreeSamplerDescriptor(crd3d::DescriptorD3D12 descriptor)
{
	m_d3d12SamplerHeap.Free(descriptor);
}