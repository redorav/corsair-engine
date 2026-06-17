#include "Graphics/CrRendering_pch.h"

#include "Graphics/GPUBuffer.h"
#include "Graphics/IDevice.h"

#include "Core/Logging/ICrDebug.h"

namespace crgfx
{
	ICrHardwareGPUBuffer::ICrHardwareGPUBuffer(crgfx::IDevice* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor) : CrGPUAutoDeletable(renderDevice)
		, m_usage(descriptor.usage)
		, m_access(descriptor.access)
		, m_dataFormat(descriptor.dataFormat)
		, m_mapped(false)
		, m_sizeBytes(descriptor.numElements* descriptor.stride)
		, m_strideBytes(descriptor.stride)
		, m_numElements(descriptor.numElements)
	{
		CrAssertMsg(m_sizeBytes > 0, "Size must be greater than zero");
		CrAssertMsg(descriptor.initialData ? descriptor.initialDataSize <= m_sizeBytes : true, "Size must be less or equal");
#if !defined(CR_CONFIG_FINAL)
		if (descriptor.name)
		{
			m_debugName = descriptor.name;
		}
#endif
	}

	// This constructor takes both a stride and a data format. While this looks like redundant information, this constructor
	// is not public, and lives here to cater for the two public-facing constructors
	CrGPUBuffer::CrGPUBuffer(crgfx::IDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, uint32_t stride, crgfx::DataFormat::T dataFormat)
		: m_usage(descriptor.usage), m_access(descriptor.access)
	{
		if (descriptor.usage & crgfx::BufferUsage::Index)
		{
			CrAssertMsg((stride == 2 && dataFormat == crgfx::DataFormat::R16_Uint) || (stride == 4 && dataFormat == crgfx::DataFormat::R32_Uint), "Invalid format for index buffer");
		}

		if (descriptor.usage & crgfx::BufferUsage::Indirect)
		{
			CrAssertMsg(descriptor.usage & (crgfx::BufferUsage::Structured | crgfx::BufferUsage::Byte), "Must specify structured or byte buffer to write to an indirect buffer");
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
};