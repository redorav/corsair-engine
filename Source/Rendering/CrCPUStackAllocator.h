#pragma once

#include "Core/CrAlignment.h"

// Manages transient memory allocated per frame for CPU resources
class CrCPUStackAllocator
{
public:

	~CrCPUStackAllocator();

	void Initialize(uint32_t size);

	template<typename T, typename... Args>
	T* AllocateObject(Args... args);

	void* Allocate(uint32_t bufferSize);

	void Reset();

protected:

	uint8_t* m_memoryBasePointer = nullptr;

	uint8_t* m_currentPointer = nullptr;

	size_t m_poolSize = 0;
};

inline void* CrCPUStackAllocator::Allocate(uint32_t size)
{
	void* bufferPointer = m_currentPointer; // Take the current pointer
	size_t alignedBufferSize = Align256(size); // Align memory as required
	CrAssertMsg(m_currentPointer + alignedBufferSize < m_memoryBasePointer + m_poolSize, "Ran out of memory in stream");
	m_currentPointer += alignedBufferSize; // Reserve as many bytes as needed by the buffer
	return bufferPointer;
}

template<typename T, typename... Args>
T* CrCPUStackAllocator::AllocateObject(Args... args)
{
	//uint32_t offset;
	void* reservedMemory = Allocate(sizeof(T)); // Allocate memory off the stream
	T* object = new(reservedMemory) T(args...); // Placement new on the memory owned by the stream
	return object;
}
