#pragma once

#include "Graphics/CrStackAllocator.h"
#include "Graphics/CrGraphicsForwardDeclarations.h"

// Manages transient memory allocated per frame for GPU resources
class CrGPUStackAllocator : public CrStackAllocator
{
public:

	CrGPUStackAllocator(crgfx::IDevice* renderDevice, const crgfx::CrHardwareGPUBufferDescriptor& descriptor);

	~CrGPUStackAllocator();

	void Begin();

	void End();

	crgfx::ICrHardwareGPUBuffer* GetHardwareGPUBuffer() const;

	crgfx::BufferUsage::T GetUsage() const;

	crgfx::MemoryAccess::T GetAccess() const;

protected:

	crgfx::IDevice* m_renderDevice = nullptr;

	crgfx::CrHardwareGPUBufferHandle m_hardwareBuffer;

	crgfx::BufferUsage::T m_bufferUsage;

	crgfx::MemoryAccess::T m_bufferAccess;
};

inline crgfx::ICrHardwareGPUBuffer* CrGPUStackAllocator::GetHardwareGPUBuffer() const
{
	return m_hardwareBuffer.get();
}

inline crgfx::BufferUsage::T CrGPUStackAllocator::GetUsage() const
{
	return m_bufferUsage;
}

inline crgfx::MemoryAccess::T CrGPUStackAllocator::GetAccess() const
{
	return m_bufferAccess;
}