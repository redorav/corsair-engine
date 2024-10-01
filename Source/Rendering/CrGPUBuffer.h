#pragma once

#include "Rendering/CrRendering.h"
#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrGPUDeletable.h"
#include "Rendering/CrVertexDescriptor.h"

#include "Core/CrHash.h"
#include "Core/SmartPointers/CrUniquePtr.h"
#include "Core/String/CrFixedString.h"
#include "Core/CrCoreForwardDeclarations.h"

#include "Core/Logging/ICrDebug.h"

struct CrHardwareGPUBufferDescriptor
{
	CrHardwareGPUBufferDescriptor(cr3d::BufferUsage::T usage, cr3d::MemoryAccess::T access, uint32_t size)
		: usage(usage), access(access), dataFormat(cr3d::DataFormat::Invalid), numElements(1), stride(size) {}

	CrHardwareGPUBufferDescriptor(cr3d::BufferUsage::T usage, cr3d::MemoryAccess::T access, uint32_t numElements, uint32_t stride)
		: usage(usage), access(access), dataFormat(cr3d::DataFormat::Invalid), numElements(numElements), stride(stride) {}

	CrHardwareGPUBufferDescriptor(cr3d::BufferUsage::T usage, cr3d::MemoryAccess::T access, uint32_t numElements, cr3d::DataFormat::T dataFormat)
		: usage(usage), access(access), dataFormat(dataFormat), numElements(numElements), stride(cr3d::DataFormats[dataFormat].dataOrBlockSize) {}

	cr3d::BufferUsage::T usage;

	cr3d::MemoryAccess::T access;

	cr3d::DataFormat::T dataFormat;

	uint32_t numElements;

	uint32_t stride;

	const uint8_t* initialData = nullptr;

	uint32_t initialDataSize = 0;

	const char* name = nullptr;
};

struct CrGPUBufferDescriptor
{
	CrGPUBufferDescriptor(cr3d::BufferUsage::T usage, cr3d::MemoryAccess::T access) : usage(usage), access(access) {}

	CrGPUBufferDescriptor(const CrGPUBufferDescriptor& descriptor) = default;

	cr3d::BufferUsage::T usage;

	cr3d::MemoryAccess::T access;

	const uint8_t* initialData = nullptr;

	uint32_t initialDataSize = 0;

	const char* name = nullptr;
};

// A ICrHardwareGPUBuffer represents a real buffer on the GPU, with allocated memory and
// properties. It can only be created from the render device. Because it is more efficient
// to allocate a relatively big buffer and suballocate from there, it is not intended to
// be created other than by lower level systems. From there one can reserve an offset and
// a size, and APIs can bind appropriately
class ICrHardwareGPUBuffer : public CrGPUAutoDeletable
{
public:

	ICrHardwareGPUBuffer(ICrRenderDevice* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor);

	virtual ~ICrHardwareGPUBuffer() {}

	void* Lock();

	void Unlock();

	virtual void* LockPS() = 0;

	virtual void UnlockPS() = 0;

	uint32_t GetSizeBytes() const { return m_sizeBytes; }

	uint32_t GetStrideBytes() const { return m_strideBytes; }

	uint32_t GetNumElements() const { return m_numElements; }

	cr3d::BufferUsage::T GetUsage() const { return m_usage; }

	cr3d::MemoryAccess::T GetAccess() const { return m_access; }

	cr3d::DataFormat::T GetDataFormat() const { return m_dataFormat; }

	bool HasUsage(cr3d::BufferUsage::T usage) const { return (m_usage & usage) != 0; }

	bool HasAccess(cr3d::MemoryAccess::T access) const { return (m_access & access) != 0; }

protected:

	cr3d::BufferUsage::T m_usage;

	cr3d::MemoryAccess::T m_access;

	cr3d::DataFormat::T m_dataFormat;

	bool m_mapped;

	uint32_t m_sizeBytes;

	uint32_t m_strideBytes;

	uint32_t m_numElements;
};

inline void* ICrHardwareGPUBuffer::Lock()
{
	CrAssertMsg(m_access != cr3d::MemoryAccess::GPUOnlyWrite && m_access != cr3d::MemoryAccess::GPUOnlyRead, "Cannot map a buffer with no CPU access");

	return LockPS();
}

inline void ICrHardwareGPUBuffer::Unlock()
{
	CrAssertMsg(m_access != cr3d::MemoryAccess::GPUOnlyWrite && m_access != cr3d::MemoryAccess::GPUOnlyRead, "Cannot unmap a buffer with no CPU access");

	return UnlockPS();
}

// This is a view of the buffer, which mainly means a size, an offset and a couple of other properties
// such as stride, data format, etc which help us tell the API how to interpret the data. Views never
// own the hardware buffer they point to, and they should be short lived. Keeping hold of or storing
// a GPU buffer is not a recommended usage. The memory field is there for the common usage of getting
// a view from a mapped buffer
class CrGPUBufferView
{
public:

	CrGPUBufferView() = default;

	CrGPUBufferView(const ICrHardwareGPUBuffer* hardwareBuffer, uint32_t numElements, uint32_t stride, uint32_t byteOffset, void* memory = nullptr)
		: m_hardwareBuffer(hardwareBuffer)
		, m_memory(memory)
		, m_byteOffset(byteOffset)
		, m_numElements(numElements)
		, m_stride(stride)
	{}

	CrGPUBufferView(const ICrHardwareGPUBuffer* hardwareBuffer, uint32_t numElements, cr3d::DataFormat::T dataFormat, uint32_t byteOffset, void* memory = nullptr)
		: m_hardwareBuffer(hardwareBuffer)
		, m_memory(memory)
		, m_byteOffset(byteOffset)
		, m_numElements(numElements)
		, m_stride(cr3d::DataFormats[dataFormat].dataOrBlockSize)
		, m_dataFormat(dataFormat)
	{}

	const ICrHardwareGPUBuffer* GetHardwareBuffer() const { return m_hardwareBuffer; }

	uint32_t GetNumElements() const { return m_numElements; }

	uint32_t GetStride() const { return m_stride; }

	uint32_t GetSize() const { return m_stride * m_numElements; }

	uint32_t GetByteOffset() const { return m_byteOffset; }

	cr3d::DataFormat::T GetFormat() const { return m_dataFormat; }

	void* GetData() const { return m_memory; }

	int32_t GetBindingIndex() const { return m_bindingIndex; }

protected:

	const ICrHardwareGPUBuffer* m_hardwareBuffer = nullptr;

	void* m_memory = nullptr;

	uint32_t m_byteOffset = 0;

	uint32_t m_numElements = 0;

	uint32_t m_stride = 0;

	cr3d::DataFormat::T m_dataFormat = cr3d::DataFormat::Invalid;

	int32_t m_bindingIndex = -1;
};

template<typename MetaType>
class CrGPUBufferViewT : public CrGPUBufferView
{
public:

	CrGPUBufferViewT()
	{
		m_bindingIndex = MetaType::index;
	}

	CrGPUBufferViewT(const ICrHardwareGPUBuffer* hardwareBuffer, uint32_t numElements, uint32_t stride, uint32_t byteOffset, void* memory = nullptr)
		: CrGPUBufferView(hardwareBuffer, numElements, stride, byteOffset, memory)
	{
		m_bindingIndex = MetaType::index;
	}

	CrGPUBufferViewT(const ICrHardwareGPUBuffer* hardwareBuffer, uint32_t numElements, cr3d::DataFormat::T dataFormat, uint32_t byteOffset, void* memory = nullptr)
		: CrGPUBufferView(hardwareBuffer, numElements, dataFormat, byteOffset, memory)
	{
		m_bindingIndex = MetaType::index;
	}

	MetaType* GetData()
	{
		return static_cast<MetaType*>(CrGPUBufferView::GetData());
	}
};

// A CrGPUBuffer holds an actual hardware buffer. It can have other convenient data in the derived classes,
// such as vertex descriptors, index sizes, binding indices, etc. It exists so we don't burden the hardware
// buffer with metadata that varies depending on usage
class CrGPUBuffer : public CrIntrusivePtrInterface
{
public:

	// Vertex buffers don't have a single fixed format but we can supply a stride
	CrGPUBuffer(ICrRenderDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, uint32_t stride)
		: CrGPUBuffer(renderDevice, descriptor, numElements, stride, cr3d::DataFormat::Invalid) {}

	CrGPUBuffer(ICrRenderDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, cr3d::DataFormat::T dataFormat)
		: CrGPUBuffer(renderDevice, descriptor, numElements, cr3d::DataFormats[dataFormat].dataOrBlockSize, dataFormat) {}

	const ICrHardwareGPUBuffer* GetHardwareBuffer() const { return m_buffer.get(); }

	uint32_t GetNumElements() const { return m_buffer->GetNumElements(); }

	uint32_t GetStride() const { return m_buffer->GetStrideBytes(); }

	cr3d::DataFormat::T GetFormat() const { return m_buffer->GetDataFormat(); }

	void* Lock();

	void Unlock();

protected:

	CrHardwareGPUBufferHandle m_buffer;

	cr3d::BufferUsage::T m_usage;

	cr3d::MemoryAccess::T m_access;

private:

	CrGPUBuffer(ICrRenderDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, uint32_t stride, cr3d::DataFormat::T dataFormat);
};

template<typename MetaType>
class CrGPUBufferType : public CrGPUBuffer
{
public:

	CrGPUBufferType(ICrRenderDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements)
		: CrGPUBuffer(renderDevice, descriptor, numElements, sizeof(MetaType))
	{
		//m_globalIndex = MetaType::index;
	}

	MetaType* Lock()
	{
		return static_cast<MetaType*>(CrGPUBuffer::Lock());
	}
};

//--------------
// Vertex Buffer
//--------------

class CrVertexBuffer : public CrGPUBuffer
{
public:

	CrVertexBuffer(ICrRenderDevice* renderDevice, cr3d::MemoryAccess::T access, const CrVertexDescriptor& vertexDescriptor, uint32_t numVertices)
		: CrGPUBuffer(renderDevice, CrGPUBufferDescriptor(
			cr3d::BufferUsage::Vertex | (access == cr3d::MemoryAccess::GPUOnlyRead ? cr3d::BufferUsage::TransferDst : cr3d::BufferUsage::None),
			access), numVertices, vertexDescriptor.GetDataSize())
		, m_vertexDescriptor(vertexDescriptor)
	{}

	const CrVertexDescriptor& GetVertexDescriptor() const { return m_vertexDescriptor; }

private:

	CrVertexDescriptor m_vertexDescriptor;
};

//-------------
// Index Buffer
//-------------

class CrIndexBuffer : public CrGPUBuffer
{
public:

	CrIndexBuffer(ICrRenderDevice* renderDevice, cr3d::MemoryAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numIndices)
		: CrGPUBuffer(renderDevice, CrGPUBufferDescriptor(
			cr3d::BufferUsage::Index | (access == cr3d::MemoryAccess::GPUOnlyRead ? cr3d::BufferUsage::TransferDst : cr3d::BufferUsage::None),
			access), numIndices, dataFormat) {}
};

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

class CrTypedBuffer : public CrGPUBuffer
{
public:

	CrTypedBuffer(ICrRenderDevice* renderDevice, cr3d::MemoryAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numElements)
		: CrGPUBuffer(renderDevice, CrGPUBufferDescriptor(cr3d::BufferUsage::Typed, access), numElements, dataFormat) {}
};