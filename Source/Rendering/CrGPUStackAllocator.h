#pragma once

#include "Core/SmartPointers/CrUniquePtr.h"
#include "Rendering/CrStackAllocator.h"

class ICrRenderDevice;
class ICrHardwareGPUBuffer;
struct CrHardwareGPUBufferDescriptor;

// Manages transient memory allocated per frame for GPU resources
class CrGPUStackAllocator : public CrStackAllocator
{
public:

	CrGPUStackAllocator(ICrRenderDevice* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor);

	virtual ~CrGPUStackAllocator() {}

	void Begin();

	void End();

	ICrHardwareGPUBuffer* GetHardwareGPUBuffer() const;

	cr3d::BufferUsage::T GetUsage() const;

	cr3d::BufferAccess::T GetAccess() const;

protected:

	ICrRenderDevice* m_renderDevice = nullptr;

	CrUniquePtr<ICrHardwareGPUBuffer> m_hardwareBuffer;

	cr3d::BufferUsage::T m_bufferUsage;

	cr3d::BufferAccess::T m_bufferAccess;
};

inline ICrHardwareGPUBuffer* CrGPUStackAllocator::GetHardwareGPUBuffer() const
{
	return m_hardwareBuffer.get();
}

inline cr3d::BufferUsage::T CrGPUStackAllocator::GetUsage() const
{
	return m_bufferUsage;
}

inline cr3d::BufferAccess::T CrGPUStackAllocator::GetAccess() const
{
	return m_bufferAccess;
}