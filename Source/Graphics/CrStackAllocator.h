#pragma once

#include "Core/CrAlignment.h"

template<typename T>
struct CrStackAllocation
{
	CrStackAllocation(T* memory, uint32_t offset) : memory(memory), offset(offset) {}

	T* memory = nullptr;
	uint32_t offset = 0;
};

// Manages transient memory allocated per frame for CPU resources
class CrStackAllocator
{
public:

	CrStackAllocation<void> Allocate(size_t sizeBytes);

	template<typename T>
	CrStackAllocation<T> Allocate(size_t count);

	CrStackAllocation<void> AllocateAligned(size_t size, size_t alignment);

protected:

	void AllocateAligned(size_t size, size_t alignment, void*& memory, uint32_t& offset);

	uint8_t* m_memoryBasePointer = nullptr;

	uint8_t* m_currentPointer = nullptr;

	size_t m_size = 0;
};

inline CrStackAllocation<void> CrStackAllocator::Allocate(size_t sizeBytes)
{
	return AllocateAligned(sizeBytes, 1);
}

template<typename T>
inline CrStackAllocation<T> CrStackAllocator::Allocate(size_t count)
{
	void* bufferPointer; uint32_t offset;
	AllocateAligned(sizeof(T) * count, 1, bufferPointer, offset);
	return CrStackAllocation<T>((T*)bufferPointer, offset);
}

inline CrStackAllocation<void> CrStackAllocator::AllocateAligned(size_t sizeBytes, size_t alignment)
{
	void* bufferPointer; uint32_t offset;
	AllocateAligned(sizeBytes, alignment, bufferPointer, offset);
	return CrStackAllocation<void>(bufferPointer, offset);
}

inline void CrStackAllocator::AllocateAligned(size_t sizeBytes, size_t alignment, void*& memory, uint32_t& offset)
{
	uint8_t* bufferPointer = CrAlignUpPow2(m_currentPointer, alignment); // Copy the current pointer, with the correct alignment
	m_currentPointer = bufferPointer + sizeBytes; // Reserve as many bytes as needed by the buffer
	CrAssertMsg(m_currentPointer < m_memoryBasePointer + m_size, "Ran out of memory in stream");

	memory = bufferPointer;
	offset = static_cast<uint32_t>(bufferPointer - m_memoryBasePointer); // Work out the offset
}