#pragma once

#include "CrRendering_pch.h"

#include "ICrGPUStackAllocator.h"

#include "ICrRenderDevice.h"

#include "CrGPUBuffer.h"

#include "Core/CrAlignment.h"

ICrGPUStackAllocator::ICrGPUStackAllocator(ICrRenderDevice* renderDevice) : m_renderDevice(renderDevice)
{

}

ICrGPUStackAllocator::~ICrGPUStackAllocator()
{

}

// Reserve a memory chunk from the constant buffer allocation pool, with size equal to the buffer size
// Then return the pointer. The calling code is responsible for filling in that memory.
GPUStackAllocation<void> ICrGPUStackAllocator::Allocate(uint32_t bufferSize)
{
	void* bufferPointer = m_currentPointer;											// Take the current pointer
	size_t alignedBufferSize = Align256(bufferSize);								// Align memory as required
	CrAssertMsg(m_currentPointer + alignedBufferSize < m_memoryBasePointer + m_poolSize, "Ran out of memory in stream!");
	uint32_t offsetInPool = static_cast<uint32_t>(m_currentPointer - m_memoryBasePointer);	// Figure out the current offset for this buffer
	m_currentPointer += alignedBufferSize;											// Reserve as many bytes as needed by the buffer
	return GPUStackAllocation<void>(bufferPointer, offsetInPool);
}

void ICrGPUStackAllocator::Init()
{
	// TODO Configurable per platform
	m_poolSize = 8 * 1024 * 1024; // 8 MB

	CrGPUBufferCreateParams params(cr3d::BufferUsage::Constant, cr3d::BufferAccess::CPUWrite, m_poolSize);
	m_renderDevice->CreateHardwareGPUBuffer(params);

	m_hardwareBuffer = CrUniquePtr<ICrHardwareGPUBuffer>(m_renderDevice->CreateHardwareGPUBuffer(params));
}

void ICrGPUStackAllocator::Begin()
{
	void* memoryPointer = BeginPS();
	m_currentPointer = m_memoryBasePointer = reinterpret_cast<uint8_t*>(memoryPointer);
}

void ICrGPUStackAllocator::End()
{
	EndPS();
}
