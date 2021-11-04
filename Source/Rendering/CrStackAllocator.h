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

	CrStackAllocation<void> Allocate(size_t size);

	CrStackAllocation<void> AllocateAligned(size_t size, size_t alignment);

protected:

	uint8_t* m_memoryBasePointer = nullptr;

	uint8_t* m_currentPointer = nullptr;

	size_t m_size = 0;
};

inline CrStackAllocation<void> CrStackAllocator::Allocate(size_t size)
{
	return AllocateAligned(size, 1);
}

inline CrStackAllocation<void> CrStackAllocator::AllocateAligned(size_t size, size_t alignment)
{
	uint8_t* bufferPointer = AlignUpPow2(m_currentPointer, alignment); // Copy the current aligned pointer
	uint32_t offset = static_cast<uint32_t>(bufferPointer - m_memoryBasePointer); // Calculate the offset
	CrAssertMsg(bufferPointer + size < m_memoryBasePointer + m_size, "Ran out of memory in stream");

	m_currentPointer = bufferPointer + size; // Update current pointer
	return CrStackAllocation<void>(bufferPointer, offset);
}
