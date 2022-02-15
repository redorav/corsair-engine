#include "CrRendering_pch.h"
#include "CrGPUBuffer.h"
#include "ICrRenderDevice.h"

#include "Core/Logging/ICrDebug.h"

ICrHardwareGPUBuffer::ICrHardwareGPUBuffer(ICrRenderDevice* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor)
	: m_renderDevice(renderDevice)
	, access(descriptor.access)
	, usage(descriptor.usage)
	, mapped(false)
{}

CrGPUBuffer::CrGPUBuffer(ICrRenderDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, uint32_t stride, cr3d::DataFormat::T dataFormat)
	: m_usage(descriptor.usage), m_access(descriptor.access), m_numElements(numElements), m_stride(stride), m_dataFormat(dataFormat)
{
	CrAssertMsg((descriptor.usage & cr3d::BufferUsage::Index) ? (stride == 2 || stride == 4) : true, "Index buffers must have a stride of 2 or 4 bytes");

	if (descriptor.existingHardwareGPUBuffer)
	{
		m_buffer = descriptor.existingHardwareGPUBuffer;
		m_memory = descriptor.memory;
		m_byteOffset = descriptor.offset;
		m_ownership = cr3d::BufferOwnership::NonOwning;
	}
	else
	{
		CrHardwareGPUBufferDescriptor hardwareGPUBufferDescriptor(descriptor.usage, descriptor.access, numElements, stride);
		hardwareGPUBufferDescriptor.dataFormat = dataFormat;

		m_buffer = renderDevice->CreateHardwareGPUBuffer(hardwareGPUBufferDescriptor);
		m_memory = nullptr;
		m_byteOffset = 0;
		m_ownership = cr3d::BufferOwnership::Owning;
	}
}

CrGPUBuffer::~CrGPUBuffer()
{
	// We only dispose of the memory if we actually own it
	if (m_ownership == cr3d::BufferOwnership::Owning)
	{
		delete m_buffer;
	}
}

void* CrGPUBuffer::Lock()
{
	if (m_ownership == cr3d::BufferOwnership::Owning)
	{
		return m_buffer->Lock();
	}
	else
	{
		return m_memory;
	}
}

void CrGPUBuffer::Unlock()
{
	if (m_ownership == cr3d::BufferOwnership::Owning)
	{
		m_buffer->Unlock();
	}
}