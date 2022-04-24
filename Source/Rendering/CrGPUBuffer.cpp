#include "CrRendering_pch.h"
#include "CrGPUBuffer.h"
#include "ICrRenderDevice.h"

#include "Core/Logging/ICrDebug.h"

ICrHardwareGPUBuffer::ICrHardwareGPUBuffer(ICrRenderDevice* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor)
	: m_renderDevice(renderDevice)
	, access(descriptor.access)
	, usage(descriptor.usage)
	, mapped(false)
	, dataFormat(descriptor.dataFormat)
	, sizeBytes(descriptor.numElements * descriptor.stride)
	, strideBytes(descriptor.stride)
{
	CrAssertMsg(sizeBytes > 0, "Size must be greater than zero");
}

// This constructor takes both a stride and a data format. While this looks like redundant information, this constructor
// is not public, and lives here to cater for the two public-facing constructors
CrGPUBuffer::CrGPUBuffer(ICrRenderDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, uint32_t stride, cr3d::DataFormat::T dataFormat)
	: m_usage(descriptor.usage), m_access(descriptor.access), m_numElements(numElements), m_stride(stride), m_dataFormat(dataFormat)
{
	if (descriptor.usage & cr3d::BufferUsage::Index)
	{
		CrAssertMsg((stride == 2 && dataFormat == cr3d::DataFormat::R16_Uint) || (stride == 4 && dataFormat == cr3d::DataFormat::R32_Uint), "Invalid format for index buffer");
	}

	if (descriptor.usage & cr3d::BufferUsage::Indirect)
	{
		CrAssertMsg(descriptor.usage & (cr3d::BufferUsage::Structured | cr3d::BufferUsage::Byte), "Must specify structured or byte buffer to write to an indirect buffer");
	}

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

		m_buffer = renderDevice->CreateHardwareGPUBufferPointer(hardwareGPUBufferDescriptor);
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