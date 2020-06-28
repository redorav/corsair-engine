#pragma once

// Manages transient memory allocated per frame for CPU resources
class CrCPUStackAllocator
{
public:

	CrCPUStackAllocator();

	~CrCPUStackAllocator();

	template<typename T, typename... Args>
	T* AllocateObject(Args... args);

	void* Allocate(uint32_t bufferSize);

	void Reset();

protected:

	uint8_t* m_memoryBasePointer = nullptr;

	uint8_t* m_currentPointer = nullptr;

	size_t m_poolSize = 0;
};

template<typename T, typename... Args>
T* CrCPUStackAllocator::AllocateObject(Args... args)
{
	//uint32_t offset;
	void* reservedMemory = Allocate(sizeof(T)); // Allocate memory off the stream
	T* object = new(reservedMemory) T(args...); // Placement new on the memory owned by the stream
	return object;
}
