#pragma once

#include "CrRendering_pch.h"

#include "CrGPUStackAllocator.h"

#include "ICrRenderDevice.h"

#include "CrGPUBuffer.h"

#include "Core/CrAlignment.h"

CrGPUStackAllocator::CrGPUStackAllocator(ICrRenderDevice* renderDevice, const CrGPUStackDescriptor& descriptor) : m_renderDevice(renderDevice)
{
	// TODO Configurable per platform
	m_poolSize = descriptor.initialSize;

	CrHardwareGPUBufferDescriptor gpuBufferDescriptor(descriptor.bufferUsage, descriptor.bufferAccess, m_poolSize);

	m_hardwareBuffer = CrUniquePtr<ICrHardwareGPUBuffer>(m_renderDevice->CreateHardwareGPUBuffer(gpuBufferDescriptor));
}

CrGPUStackAllocator::~CrGPUStackAllocator()
{

}

// Reserve a memory chunk from the constant buffer allocation pool, with size equal to the buffer size
// Then return the pointer. The calling code is responsible for filling in that memory.
GPUStackAllocation<void> CrGPUStackAllocator::Allocate(uint32_t bufferSize)
{
	void* bufferPointer = m_currentPointer;											// Take the current pointer
	size_t alignedBufferSize = AlignUp256(bufferSize);								// Align memory as required
	CrAssertMsg(m_currentPointer + alignedBufferSize < m_memoryBasePointer + m_poolSize, "Ran out of memory in stream");
	uint32_t offsetInPool = static_cast<uint32_t>(m_currentPointer - m_memoryBasePointer);	// Figure out the current offset for this buffer
	m_currentPointer += alignedBufferSize;											// Reserve as many bytes as needed by the buffer
	return GPUStackAllocation<void>(bufferPointer, offsetInPool);
}

void CrGPUStackAllocator::Begin()
{
	void* memoryPointer = m_hardwareBuffer->Lock();
	m_currentPointer = m_memoryBasePointer = reinterpret_cast<uint8_t*>(memoryPointer);
}

void CrGPUStackAllocator::End()
{
	m_hardwareBuffer->Unlock();
}
