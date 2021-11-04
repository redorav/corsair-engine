#pragma once

#include "Core/SmartPointers/CrUniquePtr.h"

class ICrRenderDevice;
class ICrHardwareGPUBuffer;
struct CrHardwareGPUBufferDescriptor;

template<typename T>
struct GPUStackAllocation
{
	GPUStackAllocation(T* memory, uint32_t offset) : memory(memory), offset(offset) {}

	const T* memory = nullptr;
	const uint32_t offset = 0;
};

// Manages transient memory allocated per frame for GPU resources
class CrGPUStackAllocator
{
public:

	CrGPUStackAllocator(ICrRenderDevice* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor);

	virtual ~CrGPUStackAllocator() {}

	template<typename T, typename... Args>
	GPUStackAllocation<T> Allocate(Args... args);

	GPUStackAllocation<void> Allocate(uint32_t bufferSize);

	void Begin();

	void End();

	ICrHardwareGPUBuffer* GetHardwareGPUBuffer() const;

	cr3d::BufferUsage::T GetUsage() const;

	cr3d::BufferAccess::T GetAccess() const;

protected:

	ICrRenderDevice* m_renderDevice = nullptr;

	CrUniquePtr<ICrHardwareGPUBuffer> m_hardwareBuffer;

	cr3d::BufferUsage::T m_bufferUsage;

	cr3d::BufferAccess::T m_bufferAccess;

	uint8_t* m_memoryBasePointer = nullptr;

	uint8_t* m_currentPointer = nullptr;

	uint32_t m_poolSize = 0;
};

inline ICrHardwareGPUBuffer* CrGPUStackAllocator::GetHardwareGPUBuffer() const
{
	return m_hardwareBuffer.get();
}

inline cr3d::BufferUsage::T CrGPUStackAllocator::GetUsage() const
{
	return m_bufferUsage;
}

inline cr3d::BufferAccess::T CrGPUStackAllocator::GetAccess() const
{
	return m_bufferAccess;
}

template<typename T, typename... Args>
GPUStackAllocation<T> CrGPUStackAllocator::Allocate(Args... args)
{
	GPUStackAllocation<T> allocation = Allocate(sizeof(T)); // Allocate memory off the stream
	new(allocation.memory) T(args...); // Placement new on the memory owned by the stream
	return allocation;
}
