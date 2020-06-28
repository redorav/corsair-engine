#pragma once

#include "Core/SmartPointers/CrUniquePtr.h"

class ICrRenderDevice;
class ICrHardwareGPUBuffer;

namespace CrGPUMemoryStreamUsage
{
	enum T
	{
		ConstantBuffer,
		VertexBuffer
	};
}

template<typename T>
struct GPUStackAllocation
{
	GPUStackAllocation(T* memory, uint32_t offset) : memory(memory), offset(offset) {}

	T* memory = nullptr;
	uint32_t offset = 0;
};

// Manages transient memory allocated per frame for GPU resources
class ICrGPUStackAllocator
{
public:

	ICrGPUStackAllocator(ICrRenderDevice* renderDevice);

	virtual ~ICrGPUStackAllocator();

	template<typename T, typename... Args>
	GPUStackAllocation<T> Allocate(Args... args);

	GPUStackAllocation<void> Allocate(uint32_t bufferSize);

	void Init();

	void Begin();

	void End();

	ICrHardwareGPUBuffer* GetHardwareGPUBuffer() const;

protected:

	virtual void InitPS(size_t size) = 0;

	virtual void* BeginPS() = 0;

	virtual void EndPS() = 0;

	ICrRenderDevice* m_renderDevice = nullptr;

	CrUniquePtr<ICrHardwareGPUBuffer> m_hardwareBuffer;

	uint8_t* m_memoryBasePointer = nullptr;

	uint8_t* m_currentPointer = nullptr;

	uint32_t m_poolSize = 0;
};

inline ICrHardwareGPUBuffer* ICrGPUStackAllocator::GetHardwareGPUBuffer() const
{
	return m_hardwareBuffer.get();
}

template<typename T, typename... Args>
GPUStackAllocation<T> ICrGPUStackAllocator::Allocate(Args... args)
{
	GPUStackAllocation<T> allocation = Allocate(sizeof(T)); // Allocate memory off the stream
	new(allocation.memory) T(args...); // Placement new on the memory owned by the stream
	return allocation;
}
