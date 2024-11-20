#pragma once

#include "Rendering/ICrRenderDevice.h"

#include "Rendering/ICrGPUSynchronization.h"

#include "CrDescriptorHeap_d3d12.h"

#include "d3d12.h"

class CrCommandQueueD3D12;
class CrTextureD3D12;

class CrRenderDeviceD3D12 final : public ICrRenderDevice
{
public:

	CrRenderDeviceD3D12(const ICrRenderSystem* renderSystem, const CrRenderDeviceDescriptor& descriptor);

	~CrRenderDeviceD3D12();

	IDXGIAdapter1* GetDXGIAdapter() const { return m_dxgiAdapter; }

	ID3D12Device* GetD3D12Device() const { return m_d3d12Device; }

	ID3D12Device10* GetD3D12Device10() const { return m_d3d12Device10; }

	// This command queue isn't really used for anything other than to build the swapchain and do other tasks
	// that the device used to do but now requires a queue. Don't use for actual rendering
	ID3D12CommandQueue* GetD3D12GraphicsCommandQueue() const { return m_d3d12GraphicsCommandQueue; }

	ID3D12RootSignature* GetD3D12GraphicsRootSignature() const { return m_d3d12GraphicsRootSignature; }

	ID3D12RootSignature* GetD3D12ComputeRootSignature() const { return m_d3d12ComputeRootSignature; }

	ID3D12CommandSignature* GetD3D12DrawIndirectCommandSignature() const { return m_d3d12DrawIndirectCommandSignature; }

	ID3D12CommandSignature* GetD3D12DrawIndexedIndirectCommandSignature() const { return m_d3d12DrawIndexedIndirectCommandSignature; }

	ID3D12CommandSignature* GetD3D12DispatchIndirectCommandSignature() const { return m_d3d12DispatchIndirectCommandSignature; }

	crd3d::DescriptorD3D12 AllocateRTVDescriptor();

	void FreeRTVDescriptor(crd3d::DescriptorD3D12 descriptor);

	crd3d::DescriptorD3D12 AllocateDSVDescriptor();

	void FreeDSVDescriptor(crd3d::DescriptorD3D12 descriptor);

	crd3d::DescriptorD3D12 AllocateSamplerDescriptor();

	void FreeSamplerDescriptor(crd3d::DescriptorD3D12 descriptor);

	void SetD3D12ObjectName(ID3D12Object* object, const char* name);

	bool GetIsEnhancedBarriersSupported() const { return m_enhancedBarriersSupported; }

private:

	//------------------
	// Resource Creation
	//------------------

	virtual ICrCommandBuffer* CreateCommandBufferPS(const CrCommandBufferDescriptor& descriptor) override;

	virtual ICrGPUFence* CreateGPUFencePS() override;

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

	virtual void FinalizeDeletionPS() override;

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

	// Heap for Render Target Views
	CrDescriptorPoolD3D12 m_rtvPool;

	// Heap for Depth Stencil Views
	CrDescriptorPoolD3D12 m_dsvPool;

	// Heap for Samplers
	CrDescriptorPoolD3D12 m_samplerPool;

	CrGPUFenceHandle m_waitIdleFence;

	ID3D12CommandQueue* m_d3d12GraphicsCommandQueue;

	ID3D12RootSignature* m_d3d12GraphicsRootSignature;

	ID3D12RootSignature* m_d3d12ComputeRootSignature;

	ID3D12CommandSignature* m_d3d12DrawIndirectCommandSignature;

	ID3D12CommandSignature* m_d3d12DrawIndexedIndirectCommandSignature;

	ID3D12CommandSignature* m_d3d12DispatchIndirectCommandSignature;

	IDXGIAdapter1* m_dxgiAdapter;

	ID3D12Device* m_d3d12Device;

	ID3D12Device1* m_d3d12Device1;

	ID3D12Device2* m_d3d12Device2;

	ID3D12Device3* m_d3d12Device3;

	ID3D12Device4* m_d3d12Device4;

	ID3D12Device5* m_d3d12Device5;

	ID3D12Device6* m_d3d12Device6;

	ID3D12Device7* m_d3d12Device7;

	ID3D12Device8* m_d3d12Device8;

	ID3D12Device9* m_d3d12Device9;

	ID3D12Device10* m_d3d12Device10;

	ID3D12Device11* m_d3d12Device11;

	ID3D12Device12* m_d3d12Device12;

	ID3D12Device13* m_d3d12Device13;

	ID3D12Device14* m_d3d12Device14;

	bool m_enhancedBarriersSupported;
};

inline crd3d::DescriptorD3D12 CrRenderDeviceD3D12::AllocateRTVDescriptor()
{
	return m_rtvPool.Allocate();
}

inline void CrRenderDeviceD3D12::FreeRTVDescriptor(crd3d::DescriptorD3D12 descriptor)
{
	CrAssertMsg(descriptor.cpuHandle.ptr != 0 && descriptor.gpuHandle.ptr != 0, "Invalid handle being returned to pool");
	m_rtvPool.Free(descriptor);
}

inline crd3d::DescriptorD3D12 CrRenderDeviceD3D12::AllocateDSVDescriptor()
{
	return m_dsvPool.Allocate();
}

inline void CrRenderDeviceD3D12::FreeDSVDescriptor(crd3d::DescriptorD3D12 descriptor)
{
	CrAssertMsg(descriptor.cpuHandle.ptr != 0 && descriptor.gpuHandle.ptr != 0, "Invalid handle being returned to pool");
	m_dsvPool.Free(descriptor);
}

inline crd3d::DescriptorD3D12 CrRenderDeviceD3D12::AllocateSamplerDescriptor()
{
	return m_samplerPool.Allocate();
}

inline void CrRenderDeviceD3D12::FreeSamplerDescriptor(crd3d::DescriptorD3D12 descriptor)
{
	m_samplerPool.Free(descriptor);
}