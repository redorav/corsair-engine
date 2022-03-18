#pragma once

#include "Rendering/CrRendering.h"
#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrGPUDeletable.h"
#include "Rendering/CrVertexDescriptor.h"

#include "Core/CrHash.h"
#include "Core/SmartPointers/CrUniquePtr.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/String/CrFixedString.h"
#include "Core/CrCoreForwardDeclarations.h"

#include "Core/Logging/ICrDebug.h"

struct CrHardwareGPUBufferDescriptor
{
	CrHardwareGPUBufferDescriptor(cr3d::BufferUsage::T usage, cr3d::MemoryAccess::T access, uint32_t size)
		: usage(usage), access(access), numElements(1), stride(size), dataFormat(cr3d::DataFormat::Invalid) {}

	CrHardwareGPUBufferDescriptor(cr3d::BufferUsage::T usage, cr3d::MemoryAccess::T access, uint32_t numElements, uint32_t stride)
		: usage(usage), access(access), numElements(numElements), stride(stride), dataFormat(cr3d::DataFormat::Invalid) {}

	CrHardwareGPUBufferDescriptor(cr3d::BufferUsage::T usage, cr3d::MemoryAccess::T access, uint32_t numElements, cr3d::DataFormat::T dataFormat)
		: usage(usage), access(access), numElements(numElements), dataFormat(dataFormat), stride(cr3d::DataFormats[dataFormat].dataOrBlockSize) {}

	cr3d::BufferUsage::T usage;

	cr3d::MemoryAccess::T access;

	cr3d::DataFormat::T dataFormat;

	uint32_t numElements;

	uint32_t stride;

	CrFixedString128 name;
};

// A ICrHardwareGPUBuffer represents a real buffer on the GPU, with allocated memory and
// properties. It can only be created from the render device. Because it is more efficient
// to allocate a relatively big buffer and suballocate from there, it is not intended to
// be created other than by lower level systems. From there one can reserve an offset and
// a size, and APIs can bind appropriately
class ICrHardwareGPUBuffer : public CrGPUDeletable
{
public:

	ICrHardwareGPUBuffer(ICrRenderDevice* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor);

	virtual ~ICrHardwareGPUBuffer() {}

	void* Lock();

	void Unlock();

	virtual void* LockPS() = 0;

	virtual void UnlockPS() = 0;

	uint32_t GetSizeBytes() const { return sizeBytes; }

	uint32_t GetStrideBytes() const { return strideBytes; }

	cr3d::DataFormat::T GetDataFormat() const { return dataFormat; }

	ICrRenderDevice* m_renderDevice;

	// TODO Make const. Once we've constructed this buffer we cannot change access or usage
	cr3d::BufferUsage::T usage;

	cr3d::MemoryAccess::T access;

	cr3d::DataFormat::T dataFormat;

	bool mapped;

	uint32_t sizeBytes;

	uint32_t strideBytes;
};

inline void* ICrHardwareGPUBuffer::Lock()
{
	CrAssertMsg(access != cr3d::MemoryAccess::GPUOnly, "Cannot map a buffer with no CPU access");

	return LockPS();
}

inline void ICrHardwareGPUBuffer::Unlock()
{
	CrAssertMsg(access != cr3d::MemoryAccess::GPUOnly, "Cannot unmap a buffer with no CPU access");

	return UnlockPS();
}

struct CrGPUBufferDescriptor
{
	CrGPUBufferDescriptor(cr3d::BufferUsage::T usage, cr3d::MemoryAccess::T access) : usage(usage), access(access) {}

	CrGPUBufferDescriptor(const CrGPUBufferDescriptor& descriptor) = default;

	ICrHardwareGPUBuffer* existingHardwareGPUBuffer = nullptr;

	void* memory = nullptr;

	uint32_t offset = 0;

	cr3d::BufferUsage::T usage;

	cr3d::MemoryAccess::T access;
};

// A CrGPUBuffer is a view into an actual hardware buffer. It contains an offset and a size that can be smaller
// than the actual buffer it points to. It can also own a buffer and dispose of it suitably.
// TODO One interesting feature we should add is a way to deallocate memory from a buffer we don't own
class CrGPUBuffer
{
public:

	// Vertex buffers don't have a single fixed format but we can supply a stride
	CrGPUBuffer(ICrRenderDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, uint32_t stride)
		: CrGPUBuffer(renderDevice, descriptor, numElements, stride, cr3d::DataFormat::Invalid) {}

	CrGPUBuffer(ICrRenderDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, cr3d::DataFormat::T dataFormat)
		: CrGPUBuffer(renderDevice, descriptor, numElements, cr3d::DataFormats[dataFormat].dataOrBlockSize, dataFormat) {}

	~CrGPUBuffer();

	const ICrHardwareGPUBuffer* GetHardwareBuffer() const;

	uint32_t GetNumElements() const;

	uint32_t GetStride() const;

	uint32_t GetSize() const;

	uint32_t GetByteOffset() const;

	cr3d::DataFormat::T GetFormat() const;

	bool HasUsage(cr3d::BufferUsage::T usage) const;

	void* Lock();

	void Unlock();

	int32_t GetGlobalIndex() const;

protected:

	ICrHardwareGPUBuffer* m_buffer;

	void* m_memory;

	uint32_t m_byteOffset;

	uint32_t m_numElements;

	uint32_t m_stride;

	int32_t m_globalIndex = -1; // Global index for binding resource as input to shader

	cr3d::BufferUsage::T m_usage;

	cr3d::MemoryAccess::T m_access;

	cr3d::BufferOwnership::T m_ownership;
	
	cr3d::DataFormat::T m_dataFormat;

private:

	CrGPUBuffer(ICrRenderDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, uint32_t stride, cr3d::DataFormat::T dataFormat);
};

inline const ICrHardwareGPUBuffer* CrGPUBuffer::GetHardwareBuffer() const
{
	return m_buffer;
}

inline uint32_t CrGPUBuffer::GetNumElements() const
{
	return m_numElements;
}

inline bool CrGPUBuffer::HasUsage(cr3d::BufferUsage::T usage) const
{
	return (m_usage | usage) != 0;
}

inline uint32_t CrGPUBuffer::GetStride() const
{
	return m_stride;
}

inline uint32_t CrGPUBuffer::GetSize() const
{
	return m_stride * m_numElements;
}

inline uint32_t CrGPUBuffer::GetByteOffset() const
{
	return m_byteOffset;
}

inline int32_t CrGPUBuffer::GetGlobalIndex() const
{
	return m_globalIndex;
}

inline cr3d::DataFormat::T CrGPUBuffer::GetFormat() const
{
	return m_dataFormat;
}

template<typename MetaType>
class CrGPUBufferType : public CrGPUBuffer
{
public:

	CrGPUBufferType(ICrRenderDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements)
		: CrGPUBuffer(renderDevice, descriptor, numElements, sizeof(MetaType))
	{
		m_globalIndex = MetaType::index;
	}

	MetaType* Lock()
	{
		return static_cast<MetaType*>(CrGPUBuffer::Lock());
	}
};

//--------------
// Vertex Buffer
//--------------

class CrVertexBufferCommon : public CrGPUBuffer
{
public:

	CrVertexBufferCommon(ICrRenderDevice* renderDevice, cr3d::MemoryAccess::T access, const CrVertexDescriptor& vertexDescriptor, uint32_t numVertices)
		: CrGPUBuffer(renderDevice, CrGPUBufferDescriptor(cr3d::BufferUsage::Vertex, access), numVertices, vertexDescriptor.GetDataSize())
		, m_vertexDescriptor(vertexDescriptor)
	{}

	const CrVertexDescriptor& GetVertexDescriptor() const { return m_vertexDescriptor; }

private:

	CrVertexDescriptor m_vertexDescriptor;
};

//-------------
// Index Buffer
//-------------

class CrIndexBufferCommon : public CrGPUBuffer
{
public:

	CrIndexBufferCommon(ICrRenderDevice* renderDevice, cr3d::MemoryAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numIndices)
		: CrGPUBuffer(renderDevice, CrGPUBufferDescriptor(cr3d::BufferUsage::Index, access), numIndices, dataFormat) {}
};

using CrIndexBufferSharedHandle = CrSharedPtr<CrIndexBufferCommon>;

//----------------
// Constant Buffer
//----------------

/*class CrConstantBufferCommon : public ICrGPUBuffer
{
public:

	CrConstantBufferCommon(ICrRenderDevice* renderDevice, uint32_t numIndices) 
		: ICrGPUBuffer(renderDevice, cr3d::BufferUsage::Constant, cr3d::BufferAccess::CPUWrite, numIndices, dataFormat == cr3d::DataFormat::R16_Uint ? 2 : 4) {}
};

: ICrGPUBuffer(renderDevice, cr3d::BufferUsage::Vertex, cr3d::BufferAccess::CPUWrite, numVertices, vertexDescriptor.GetDataSize())*/

//------------------
// Structured Buffer
//------------------

template<typename Metadata>
class CrStructuredBuffer : public CrGPUBufferType<Metadata>
{
public:

	CrStructuredBuffer(ICrRenderDevice* renderDevice, cr3d::MemoryAccess::T bufferAccess, uint32_t numElements) 
		: CrGPUBufferType<Metadata>(renderDevice, CrGPUBufferDescriptor(cr3d::BufferUsage::Structured, bufferAccess), numElements) {}

	Metadata* Lock()
	{
		return static_cast<Metadata*>(CrGPUBuffer::Lock());
	}
};

//------------
// Data Buffer
//------------

class CrDataBuffer : public CrGPUBuffer
{
public:

	CrDataBuffer(ICrRenderDevice* renderDevice, cr3d::MemoryAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numElements)
		: CrGPUBuffer(renderDevice, CrGPUBufferDescriptor(cr3d::BufferUsage::Data, access), numElements, dataFormat) {}
};