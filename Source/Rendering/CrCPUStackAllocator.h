#pragma once

#include "Core/CrAlignment.h"

// Manages transient memory allocated per frame for CPU resources
class CrCPUStackAllocator
{
public:

	~CrCPUStackAllocator();

	void Initialize(size_t size);

	template<typename T, typename... Args>
	T* AllocateObject(Args... args);

	void* Allocate(size_t size);

	void* AllocateAligned(size_t size, size_t alignment);

	void Reset();

protected:

	uint8_t* m_memoryBasePointer = nullptr;

	uint8_t* m_currentPointer = nullptr;

	size_t m_poolSize = 0;
};

inline void* CrCPUStackAllocator::Allocate(size_t size)
{
	return AllocateAligned(size, 1);
}

inline void* CrCPUStackAllocator::AllocateAligned(size_t size, size_t alignment)
{
	uint8_t* bufferPointer = AlignUpPow2(m_currentPointer, alignment); // Take the current aligned pointer
	CrAssertMsg(bufferPointer + size < m_memoryBasePointer + m_poolSize, "Ran out of memory in stream");
	m_currentPointer = bufferPointer + size; // Reserve as many bytes as needed by the buffer
	return bufferPointer;
}

template<typename T, typename... Args>
T* CrCPUStackAllocator::AllocateObject(Args... args)
{
	void* reservedMemory = Allocate(sizeof(T)); // Allocate memory off the stream
	T* object = new(reservedMemory) T(args...); // Placement new on the memory owned by the stream
	return object;
}
