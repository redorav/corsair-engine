#pragma once

#include "Graphics/CrRendering.h"
#include "Graphics/CrGraphicsForwardDeclarations.h"
#include "Graphics/CrGPUDeletable.h"
#include "Graphics/CrVertexDescriptor.h"

#include "Core/CrHash.h"
#include "Core/CrCoreForwardDeclarations.h"

#include "Core/Logging/ICrDebug.h"

#include "crstl/fixed_string.h"

struct CrHardwareGPUBufferDescriptor
{
	CrHardwareGPUBufferDescriptor(crgfx::BufferUsage::T usage, crgfx::MemoryAccess::T access, uint32_t size)
		: usage(usage), access(access), dataFormat(crgfx::DataFormat::Invalid), numElements(1), stride(size) {}

	CrHardwareGPUBufferDescriptor(crgfx::BufferUsage::T usage, crgfx::MemoryAccess::T access, uint32_t numElements, uint32_t stride)
		: usage(usage), access(access), dataFormat(crgfx::DataFormat::Invalid), numElements(numElements), stride(stride) {}

	CrHardwareGPUBufferDescriptor(crgfx::BufferUsage::T usage, crgfx::MemoryAccess::T access, uint32_t numElements, crgfx::DataFormat::T dataFormat)
		: usage(usage), access(access), dataFormat(dataFormat), numElements(numElements), stride(crgfx::DataFormats[dataFormat].dataOrBlockSize) {}

	crgfx::BufferUsage::T usage;

	crgfx::MemoryAccess::T access;

	crgfx::DataFormat::T dataFormat;

	uint32_t numElements;

	uint32_t stride;

	const uint8_t* initialData = nullptr;

	uint32_t initialDataSize = 0;

	const char* name = nullptr;
};

struct CrGPUBufferDescriptor
{
	CrGPUBufferDescriptor(crgfx::BufferUsage::T usage, crgfx::MemoryAccess::T access) : usage(usage), access(access) {}

	CrGPUBufferDescriptor(const CrGPUBufferDescriptor& descriptor) = default;

	crgfx::BufferUsage::T usage;

	crgfx::MemoryAccess::T access;

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

	ICrHardwareGPUBuffer(crgfx::IDevice* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor);

	virtual ~ICrHardwareGPUBuffer() {}

	void* Lock();

	void Unlock();

	virtual void* LockPS() = 0;

	virtual void UnlockPS() = 0;

	uint32_t GetSizeBytes() const { return m_sizeBytes; }

	uint32_t GetStrideBytes() const { return m_strideBytes; }

	uint32_t GetNumElements() const { return m_numElements; }

	crgfx::BufferUsage::T GetUsage() const { return m_usage; }

	crgfx::MemoryAccess::T GetAccess() const { return m_access; }

	crgfx::DataFormat::T GetDataFormat() const { return m_dataFormat; }

	bool HasUsage(crgfx::BufferUsage::T usage) const { return (m_usage & usage) != 0; }

	bool HasAccess(crgfx::MemoryAccess::T access) const { return (m_access & access) != 0; }

#if !defined(CR_CONFIG_FINAL)

	const char* GetDebugName() const
	{
		return m_debugName.c_str();
	}

#endif

protected:

	crgfx::BufferUsage::T m_usage;

	crgfx::MemoryAccess::T m_access;

	crgfx::DataFormat::T m_dataFormat;

	bool m_mapped;

	uint32_t m_sizeBytes;

	uint32_t m_strideBytes;

	uint32_t m_numElements;

#if !defined(CR_CONFIG_FINAL)

	crstl::fixed_string128 m_debugName;

#endif
};

inline void* ICrHardwareGPUBuffer::Lock()
{
	CrAssertMsg(m_access != crgfx::MemoryAccess::GPUOnlyWrite && m_access != crgfx::MemoryAccess::GPUOnlyRead, "Cannot map a buffer with no CPU access");

	return LockPS();
}

inline void ICrHardwareGPUBuffer::Unlock()
{
	CrAssertMsg(m_access != crgfx::MemoryAccess::GPUOnlyWrite && m_access != crgfx::MemoryAccess::GPUOnlyRead, "Cannot unmap a buffer with no CPU access");

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

	CrGPUBufferView(const ICrHardwareGPUBuffer* hardwareBuffer, uint32_t numElements, crgfx::DataFormat::T dataFormat, uint32_t byteOffset, void* memory = nullptr)
		: m_hardwareBuffer(hardwareBuffer)
		, m_memory(memory)
		, m_byteOffset(byteOffset)
		, m_numElements(numElements)
		, m_stride(crgfx::DataFormats[dataFormat].dataOrBlockSize)
		, m_dataFormat(dataFormat)
	{}

	const ICrHardwareGPUBuffer* GetHardwareBuffer() const { return m_hardwareBuffer; }

	uint32_t GetNumElements() const { return m_numElements; }

	uint32_t GetStride() const { return m_stride; }

	uint32_t GetSizeBytes() const { return m_stride * m_numElements; }

	uint32_t GetByteOffset() const { return m_byteOffset; }

	crgfx::DataFormat::T GetFormat() const { return m_dataFormat; }

	void* GetData() const { return m_memory; }

	int32_t GetBindingIndex() const { return m_bindingIndex; }

protected:

	const ICrHardwareGPUBuffer* m_hardwareBuffer = nullptr;

	void* m_memory = nullptr;

	uint32_t m_byteOffset = 0;

	uint32_t m_numElements = 0;

	uint32_t m_stride = 0;

	crgfx::DataFormat::T m_dataFormat = crgfx::DataFormat::Invalid;

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

	CrGPUBufferViewT(const ICrHardwareGPUBuffer* hardwareBuffer, uint32_t numElements, crgfx::DataFormat::T dataFormat, uint32_t byteOffset, void* memory = nullptr)
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
class CrGPUBuffer : public crstl::intrusive_ptr_interface_delete
{
public:

	// Vertex buffers don't have a single fixed format but we can supply a stride
	CrGPUBuffer(crgfx::IDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, uint32_t stride)
		: CrGPUBuffer(renderDevice, descriptor, numElements, stride, crgfx::DataFormat::Invalid) {}

	CrGPUBuffer(crgfx::IDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, crgfx::DataFormat::T dataFormat)
		: CrGPUBuffer(renderDevice, descriptor, numElements, crgfx::DataFormats[dataFormat].dataOrBlockSize, dataFormat) {}

	const ICrHardwareGPUBuffer* GetHardwareBuffer() const { return m_buffer.get(); }

	uint32_t GetNumElements() const { return m_buffer->GetNumElements(); }

	uint32_t GetStride() const { return m_buffer->GetStrideBytes(); }

	crgfx::DataFormat::T GetFormat() const { return m_buffer->GetDataFormat(); }

	void* Lock();

	void Unlock();

protected:

	CrHardwareGPUBufferHandle m_buffer;

	crgfx::BufferUsage::T m_usage;

	crgfx::MemoryAccess::T m_access;

private:

	CrGPUBuffer(crgfx::IDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements, uint32_t stride, crgfx::DataFormat::T dataFormat);
};

template<typename MetaType>
class CrGPUBufferType : public CrGPUBuffer
{
public:

	CrGPUBufferType(crgfx::IDevice* renderDevice, const CrGPUBufferDescriptor& descriptor, uint32_t numElements)
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

	CrVertexBuffer(crgfx::IDevice* renderDevice, crgfx::MemoryAccess::T access, const CrVertexDescriptor& vertexDescriptor, uint32_t numVertices)
		: CrGPUBuffer(renderDevice, CrGPUBufferDescriptor(
			crgfx::BufferUsage::Vertex | (access == crgfx::MemoryAccess::GPUOnlyRead ? crgfx::BufferUsage::TransferDst : crgfx::BufferUsage::None),
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

	CrIndexBuffer(crgfx::IDevice* renderDevice, crgfx::MemoryAccess::T access, crgfx::DataFormat::T dataFormat, uint32_t numIndices)
		: CrGPUBuffer(renderDevice, CrGPUBufferDescriptor(
			crgfx::BufferUsage::Index | (access == crgfx::MemoryAccess::GPUOnlyRead ? crgfx::BufferUsage::TransferDst : crgfx::BufferUsage::None),
			access), numIndices, dataFormat) {}
};

//----------------
// Constant Buffer
//----------------

/*class CrConstantBufferCommon : public ICrGPUBuffer
{
public:

	CrConstantBufferCommon(crgfx::ICrRenderDevice* renderDevice, uint32_t numIndices) 
		: ICrGPUBuffer(renderDevice, crgfx::BufferUsage::Constant, crgfx::BufferAccess::CPUWrite, numIndices, dataFormat == crgfx::DataFormat::R16_Uint ? 2 : 4) {}
};

: ICrGPUBuffer(renderDevice, crgfx::BufferUsage::Vertex, crgfx::BufferAccess::CPUWrite, numVertices, vertexDescriptor.GetDataSize())*/

//------------------
// Structured Buffer
//------------------

template<typename Metadata>
class CrStructuredBuffer : public CrGPUBufferType<Metadata>
{
public:

	CrStructuredBuffer(crgfx::IDevice* renderDevice, crgfx::MemoryAccess::T bufferAccess, uint32_t numElements)
		: CrGPUBufferType<Metadata>(renderDevice, CrGPUBufferDescriptor(crgfx::BufferUsage::Structured, bufferAccess), numElements) {}

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

	CrTypedBuffer(crgfx::IDevice* renderDevice, crgfx::MemoryAccess::T access, crgfx::DataFormat::T dataFormat, uint32_t numElements)
		: CrGPUBuffer(renderDevice, CrGPUBufferDescriptor(crgfx::BufferUsage::Typed, access), numElements, dataFormat) {}
};