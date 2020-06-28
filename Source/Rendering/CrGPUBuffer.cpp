#include "CrRendering_pch.h"
#include "CrGPUBuffer.h"
#include "ICrRenderDevice.h"

#include "Core/Logging/ICrDebug.h"

CrGPUBufferCreateParams::CrGPUBufferCreateParams(cr3d::BufferUsage::T usage, cr3d::BufferAccess::T access, uint32_t numElements, uint32_t stride)
	: usage(usage), access(access), numElements(numElements), stride(stride), size(numElements * stride)
{

}

CrGPUBufferCreateParams::CrGPUBufferCreateParams(cr3d::BufferUsage::T usage, cr3d::BufferAccess::T access, uint32_t size)
	: usage(usage), access(access), size(size), numElements(1), stride(size)
{

}

ICrHardwareGPUBuffer::ICrHardwareGPUBuffer(const CrGPUBufferCreateParams& params)
{
	access = params.access;
	usage = params.usage;
	mapped = false;
}

CrGPUBuffer::CrGPUBuffer(ICrRenderDevice* renderDevice, const CrGPUBufferCreateParams& params)
	: m_usage(params.usage), m_access(params.access), m_numElements(params.numElements), m_stride(params.stride)
{
	CrAssertMsg((params.usage & cr3d::BufferUsage::Index) ? (params.stride == 2 || params.stride == 4) : true, "Index buffers must have a stride of 2 or 4 bytes!");

	if (params.existingHardwareGPUBuffer)
	{
		m_buffer = params.existingHardwareGPUBuffer;
		m_memory = params.memory;
		m_byteOffset = params.offset;
		m_ownership = cr3d::BufferOwnership::NonOwning;
	}
	else
	{
		m_buffer = renderDevice->CreateHardwareGPUBuffer(params);
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

CrVertexDescriptor::CrVertexDescriptor(std::initializer_list<cr3d::DataFormat::T> l) : m_dataSize(0)
{
	std::initializer_list<cr3d::DataFormat::T>::iterator it = l.begin();
	while (it != l.end())
	{
		AddVertexAttribute(*it);
		++it;
	}
}

CrVertexDescriptor::CrVertexDescriptor() : m_dataSize(0)
{

}

void CrVertexDescriptor::AddVertexAttribute(cr3d::DataFormat::T format)
{
	m_dataSize += cr3d::DataFormats[format].dataOrBlockSize;
	m_vertexAttributes.push_back(format);
}

uint32_t CrVertexDescriptor::GetNumAttributes() const
{
	return (uint32_t)m_vertexAttributes.size();
}

uint32_t CrVertexDescriptor::GetDataSize() const
{
	return m_dataSize;
}

const cr3d::DataFormatInfo& CrVertexDescriptor::GetVertexInfo(uint32_t attributeIndex) const
{
	return cr3d::DataFormats[m_vertexAttributes[attributeIndex]];
}

CrHash CrVertexDescriptor::ComputeHash()
{
	CrHash hash(m_vertexAttributes.data(), sizeof(cr3d::DataFormat::T) * m_vertexAttributes.size());
	return hash;
}