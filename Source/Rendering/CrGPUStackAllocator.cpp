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
	m_size = gpuBufferDescriptor.numElements * gpuBufferDescriptor.stride;
	m_hardwareBuffer = CrUniquePtr<ICrHardwareGPUBuffer>(m_renderDevice->CreateHardwareGPUBuffer(gpuBufferDescriptor));
}

void CrGPUStackAllocator::Begin()
{
	CrAssertMsg(m_currentPointer == nullptr, "Did you call end?");
	void* memoryPointer = m_hardwareBuffer->Lock();
	m_currentPointer = m_memoryBasePointer = reinterpret_cast<uint8_t*>(memoryPointer);
}

void CrGPUStackAllocator::End()
{
	CrAssertMsg(m_currentPointer != nullptr, "Did you call begin?");
	m_hardwareBuffer->Unlock();
	m_currentPointer = nullptr;
}
