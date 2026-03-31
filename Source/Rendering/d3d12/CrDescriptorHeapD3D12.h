#pragma once

#include "CrD3D12.h"

#include "crstl/vector.h"

class CrRenderDeviceD3D12;

struct CrDescriptorHeapDescriptor
{
	const char* name = "";
	uint32_t numDescriptors = 0;
	D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
	D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
};

class CrDescriptorHeapD3D12
{
public:

	static uint32_t GetMaxDescriptorsPerHeap(const CrDescriptorHeapDescriptor& descriptor);

	void Initialize(CrRenderDeviceD3D12* d3d12RenderDevice, const CrDescriptorHeapDescriptor& descriptor);

	D3D12_CPU_DESCRIPTOR_HANDLE GetHeapStartCPU() const
	{
		return m_heapStartCPU;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetHeapStartGPU() const
	{
		return m_heapStartGPU;
	}

	uint32_t GetDescriptorStride() const { return m_descriptorStride; }

	ID3D12DescriptorHeap* GetD3D12DescriptorHeap() const { return m_descriptorHeap; }

private:

	ID3D12Device* m_d3d12Device;

	// Size of each descriptor
	uint32_t m_descriptorStride;

	D3D12_CPU_DESCRIPTOR_HANDLE m_heapStartCPU;

	D3D12_GPU_DESCRIPTOR_HANDLE m_heapStartGPU;

	ID3D12DescriptorHeap* m_descriptorHeap;
};

// Pool of descriptors that can be reused. Putting a descriptor back in the pool implicitly assumes that the descriptor can
// be reused, which is often the case for non-shader-visible descriptors. Therefore this caters well for the resource view
// use case, e.g. we create a render target view associated to a resource, and its lifetime is the same as that of the resource
class CrDescriptorPoolD3D12
{
public:

	void Initialize(CrRenderDeviceD3D12* d3d12RenderDevice, const CrDescriptorHeapDescriptor& descriptor);

	crd3d::DescriptorD3D12 Allocate();

	void Free(crd3d::DescriptorD3D12 descriptor);

	const CrDescriptorHeapD3D12& GetDescriptorHeap() const { return m_descriptorHeap; }

private:

	CrDescriptorHeapD3D12 m_descriptorHeap;

	crstl::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_availableCPUDescriptors;

	crstl::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_availableGPUDescriptors;
};

// A vector of CPU descriptors that we'll copy into from different sources. At the end we do the entire copy from CPU-visible to shader visible descriptors
// This is faster than trying to copy them in place as we create them
class CrCPUDescriptorScratchD3D12
{
public:

	void Initialize(size_t count);

	size_t Allocate(size_t count);

	void Reset();

	size_t Size()
	{
		return m_currentOffset;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE& operator [] (size_t i)
	{
		return m_descriptors[i];
	}

private:

	size_t m_currentOffset;

	crstl::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_descriptors;
};

// Stream of descriptors that one never returns descriptors to. This is meant to be used throughout the frame, and it's where
// shader-visible descriptors go. Once the frame has been consumed by the GPU, all those descriptors can be reused
class CrDescriptorStreamD3D12
{
public:

	void Initialize(CrRenderDeviceD3D12* d3d12RenderDevice, const CrDescriptorHeapDescriptor& descriptor);

	CrDescriptorStreamD3D12();

	crd3d::DescriptorD3D12 Allocate(uint32_t count);

	void Reset();

	const CrDescriptorHeapD3D12& GetDescriptorHeap() const { return m_descriptorHeap; }

	uint32_t GetNumDescriptors() const { return m_currentDescriptor; }

private:

	CrDescriptorHeapD3D12 m_descriptorHeap;

	uint32_t m_currentDescriptor;
};
