#pragma once

#include "CrRendering_pch.h"

#include "CrGPUStackAllocator.h"

#include "ICrRenderDevice.h"

#include "CrGPUBuffer.h"

#include "Core/CrAlignment.h"

CrGPUStackAllocator::CrGPUStackAllocator(ICrRenderDevice* renderDevice, const CrHardwareGPUBufferDescriptor& gpuBufferDescriptor) 
	: m_renderDevice(renderDevice)
	, m_bufferUsage(gpuBufferDescriptor.usage)
	, m_bufferAccess(gpuBufferDescriptor.access)
{
	m_poolSize = gpuBufferDescriptor.numElements * gpuBufferDescriptor.stride;
	m_hardwareBuffer = CrUniquePtr<ICrHardwareGPUBuffer>(m_renderDevice->CreateHardwareGPUBuffer(gpuBufferDescriptor));
}

// Reserve a memory chunk from the constant buffer allocation pool, with size equal to the buffer size
// Then return the pointer. The calling code is responsible for filling in that memory.
GPUStackAllocation<void> CrGPUStackAllocator::Allocate(uint32_t bufferSize)
{
	uint8_t* bufferPointer = AlignUp256(m_currentPointer); // Copy the current pointer, with the correct alignment
	m_currentPointer = bufferPointer + bufferSize;         // Update current pointer
	CrAssertMsg(m_currentPointer < m_memoryBasePointer + m_poolSize, "Ran out of memory in stream"); // Make sure pointer didn't go over
	uint32_t offsetInPool = static_cast<uint32_t>(bufferPointer - m_memoryBasePointer); // Work out the offset
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
