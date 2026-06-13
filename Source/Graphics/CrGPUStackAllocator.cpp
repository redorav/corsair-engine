#include "Graphics/CrRendering_pch.h"

#include "Graphics/CrGPUStackAllocator.h"
#include "Graphics/IDevice.h"
#include "Graphics/CrGPUBuffer.h"

#include "Core/CrAlignment.h"

CrGPUStackAllocator::CrGPUStackAllocator(crgfx::IDevice* renderDevice, const CrHardwareGPUBufferDescriptor& gpuBufferDescriptor)
	: m_renderDevice(renderDevice)
	, m_bufferUsage(gpuBufferDescriptor.usage)
	, m_bufferAccess(gpuBufferDescriptor.access)
{
	m_size = gpuBufferDescriptor.numElements * gpuBufferDescriptor.stride;
	m_hardwareBuffer = m_renderDevice->CreateHardwareGPUBuffer(gpuBufferDescriptor);
}

CrGPUStackAllocator::~CrGPUStackAllocator()
{
	if (m_currentPointer)
	{
		End();
	}
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
