#pragma once

#include "Graphics/CrStackAllocator.h"
#include "Graphics/CrRenderingForwardDeclarations.h"

class ICrRenderDevice;
class ICrHardwareGPUBuffer;
struct CrHardwareGPUBufferDescriptor;

// Manages transient memory allocated per frame for GPU resources
class CrGPUStackAllocator : public CrStackAllocator
{
public:

	CrGPUStackAllocator(ICrRenderDevice* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor);

	~CrGPUStackAllocator();

	void Begin();

	void End();

	ICrHardwareGPUBuffer* GetHardwareGPUBuffer() const;

	crgfx::BufferUsage::T GetUsage() const;

	crgfx::MemoryAccess::T GetAccess() const;

protected:

	ICrRenderDevice* m_renderDevice = nullptr;

	CrHardwareGPUBufferHandle m_hardwareBuffer;

	crgfx::BufferUsage::T m_bufferUsage;

	crgfx::MemoryAccess::T m_bufferAccess;
};

inline ICrHardwareGPUBuffer* CrGPUStackAllocator::GetHardwareGPUBuffer() const
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