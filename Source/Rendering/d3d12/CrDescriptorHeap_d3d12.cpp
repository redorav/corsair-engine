#include "CrRendering_pch.h"

#include "CrRenderDevice_d3d12.h"

#include "Core/Logging/ICrDebug.h"
#include "Core/CrAlignment.h"

#include "Math/CrMath.h"

uint32_t CrDescriptorHeapD3D12::GetMaxDescriptorsPerHeap(const CrDescriptorHeapDescriptor& descriptor)
{
	// https://docs.microsoft.com/en-us/windows/win32/direct3d12/hardware-support?redirectedfrom=MSDN
	if (descriptor.flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
	{
		// Only samplers and CBV_SRV_UAV can go into shader visible heaps
		if (descriptor.type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			return 2048;
		}
		else
		{
			return 1000000;
		}
	}
	else
	{
		return 0xffffffff; // No limit if heap is non shader visible
	}
}

void CrDescriptorHeapD3D12::Initialize(CrRenderDeviceD3D12* d3d12RenderDevice, const CrDescriptorHeapDescriptor& heapDescriptor)
{
	m_d3d12Device = d3d12RenderDevice->GetD3D12Device();

	m_descriptorStride = m_d3d12Device->GetDescriptorHandleIncrementSize(heapDescriptor.type);

	// We have an array of heaps, because heaps have a maximum amount of descriptors they can hold depending on the type
	// That way, if we request N descriptors and there is a maximum M < N per heap, we'll need to create ceil(N / M) heaps

	D3D12_DESCRIPTOR_HEAP_DESC d3d12HeapDescriptor = {};
	d3d12HeapDescriptor.Type = heapDescriptor.type;
	d3d12HeapDescriptor.NumDescriptors = heapDescriptor.numDescriptors;
	d3d12HeapDescriptor.Flags = heapDescriptor.flags;
	d3d12HeapDescriptor.NodeMask = 0; // TODO Multi-GPU
	HRESULT hResult = m_d3d12Device->CreateDescriptorHeap(&d3d12HeapDescriptor, IID_PPV_ARGS(&m_descriptorHeap));
	CrAssertMsg(SUCCEEDED(hResult), "Error creating descriptor heap");

	d3d12RenderDevice->SetD3D12ObjectName(m_descriptorHeap, heapDescriptor.name);

	m_heapStart.cpuHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

	if (heapDescriptor.flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
	{
		m_heapStart.gpuHandle = m_descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	}
	else
	{
		m_heapStart.gpuHandle.ptr = (UINT64)-1;
	}
}

void CrDescriptorPoolD3D12::Initialize(CrRenderDeviceD3D12* renderDeviceD3D12, const CrDescriptorHeapDescriptor& descriptor)
{
	uint32_t maxDescriptors = CrMin(descriptor.numDescriptors, CrDescriptorHeapD3D12::GetMaxDescriptorsPerHeap(descriptor));

	CrAssertMsg(descriptor.numDescriptors <= maxDescriptors, "Exceeded maximum numbers of descriptors");

	CrDescriptorHeapDescriptor heapDescriptor;
	heapDescriptor.name = descriptor.name;
	heapDescriptor.numDescriptors = maxDescriptors;
	heapDescriptor.type = descriptor.type;
	heapDescriptor.flags = descriptor.flags;
	m_descriptorHeap.Initialize(renderDeviceD3D12, heapDescriptor);

	crd3d::DescriptorD3D12 heapStart = m_descriptorHeap.GetHeapStart();

	m_availableDescriptors.resize(maxDescriptors);
	for (uint32_t j = 0; j < m_availableDescriptors.size(); ++j)
	{
		m_availableDescriptors[j].cpuHandle.ptr = heapStart.cpuHandle.ptr + j * m_descriptorHeap.GetDescriptorStride();
		m_availableDescriptors[j].gpuHandle.ptr = heapStart.gpuHandle.ptr + j * m_descriptorHeap.GetDescriptorStride();
	}
}

crd3d::DescriptorD3D12 CrDescriptorPoolD3D12::Allocate()
{
	crd3d::DescriptorD3D12 descriptor = m_availableDescriptors.back();
	m_availableDescriptors.pop_back();
	return descriptor;
}

void CrDescriptorPoolD3D12::Free(crd3d::DescriptorD3D12 descriptor)
{
	m_availableDescriptors.push_back(descriptor);
}

CrDescriptorStreamD3D12::CrDescriptorStreamD3D12()
	: m_currentDescriptor(0)
{
	
}

void CrDescriptorStreamD3D12::Initialize(CrRenderDeviceD3D12* renderDeviceD3D12, const CrDescriptorHeapDescriptor& descriptor)
{
	uint32_t maxDescriptors = CrMin(descriptor.numDescriptors, CrDescriptorHeapD3D12::GetMaxDescriptorsPerHeap(descriptor));

	CrDescriptorHeapDescriptor heapDescriptor;
	heapDescriptor.name           = descriptor.name;
	heapDescriptor.numDescriptors = maxDescriptors;
	heapDescriptor.type           = descriptor.type;
	heapDescriptor.flags          = descriptor.flags;
	m_descriptorHeap.Initialize(renderDeviceD3D12, heapDescriptor);
}

crd3d::DescriptorD3D12 CrDescriptorStreamD3D12::Allocate(uint32_t count)
{
	crd3d::DescriptorD3D12 descriptor = m_descriptorHeap.GetHeapStart();
	descriptor.cpuHandle.ptr += m_currentDescriptor * m_descriptorHeap.GetDescriptorStride();
	descriptor.gpuHandle.ptr += m_currentDescriptor * m_descriptorHeap.GetDescriptorStride();
	m_currentDescriptor += count;
	return descriptor;
}

void CrDescriptorStreamD3D12::Reset()
{
	m_currentDescriptor = 0;
}
