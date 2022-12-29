#pragma once

#include "Rendering/CrStackAllocator.h"
#include "Rendering/CrRenderingForwardDeclarations.h"

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

	cr3d::BufferUsage::T GetUsage() const;

	cr3d::MemoryAccess::T GetAccess() const;

protected:

	ICrRenderDevice* m_renderDevice = nullptr;

	CrHardwareGPUBufferHandle m_hardwareBuffer;

	cr3d::BufferUsage::T m_bufferUsage;

	cr3d::MemoryAccess::T m_bufferAccess;
};

inline ICrHardwareGPUBuffer* CrGPUStackAllocator::GetHardwareGPUBuffer() const
{
	return m_hardwareBuffer.get();
}

inline cr3d::BufferUsage::T CrGPUStackAllocator::GetUsage() const
{
	return m_bufferUsage;
}

inline cr3d::MemoryAccess::T CrGPUStackAllocator::GetAccess() const
{
	return m_bufferAccess;
}