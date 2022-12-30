#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrGPUBuffer.h"
#include "Rendering/ICrRenderDevice.h"

#include "Core/Logging/ICrDebug.h"

ICrHardwareGPUBuffer::ICrHardwareGPUBuffer(ICrRenderDevice* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor) : CrGPUDeletable(renderDevice)
	, m_usage(descriptor.usage)
	, m_access(descriptor.access)
	, m_dataFormat(descriptor.dataFormat)
	, m_mapped(false)
	, m_sizeBytes(descriptor.numElements * descriptor.stride)
	, m_strideBytes(descriptor.stride)
	, m_numElements(descriptor.numElements)
{
	CrAssertMsg(m_sizeBytes > 0, "Size must be greater than zero");
	CrAssertMsg(descriptor.initialData ? descriptor.initialDataSize <= m_sizeBytes : true, "Size must be less or equal");
}

// This constructor takes both a stride and a data format. While this looks like redundant information, this constructor
// is not public, and lives here to cater for the two public-facing constructors
CrGPUBuffer::CrGPUBuffer(ICrRenderDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, uint32_t stride, cr3d::DataFormat::T dataFormat)
	: m_usage(descriptor.usage), m_access(descriptor.access)
{
	if (descriptor.usage & cr3d::BufferUsage::Index)
	{
		CrAssertMsg((stride == 2 && dataFormat == cr3d::DataFormat::R16_Uint) || (stride == 4 && dataFormat == cr3d::DataFormat::R32_Uint), "Invalid format for index buffer");
	}

	if (descriptor.usage & cr3d::BufferUsage::Indirect)
	{
		CrAssertMsg(descriptor.usage & (cr3d::BufferUsage::Structured | cr3d::BufferUsage::Byte), "Must specify structured or byte buffer to write to an indirect buffer");
	}

	CrHardwareGPUBufferDescriptor hardwareGPUBufferDescriptor(descriptor.usage, descriptor.access, numElements, stride);
	hardwareGPUBufferDescriptor.dataFormat = dataFormat;
	hardwareGPUBufferDescriptor.initialData = descriptor.initialData;
	hardwareGPUBufferDescriptor.initialDataSize = descriptor.initialDataSize;
	hardwareGPUBufferDescriptor.name = descriptor.name;
	m_buffer = renderDevice->CreateHardwareGPUBuffer(hardwareGPUBufferDescriptor);
}

void* CrGPUBuffer::Lock()
{
	return m_buffer->Lock();
}

void CrGPUBuffer::Unlock()
{
	m_buffer->Unlock();
}