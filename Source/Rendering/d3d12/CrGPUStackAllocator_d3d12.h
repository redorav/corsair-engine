#pragma once

#include "ICrGPUStackAllocator.h"

class CrGPUStackAllocatorD3D12 final : public ICrGPUStackAllocator
{
public:

	CrGPUStackAllocatorD3D12(ICrRenderDevice* renderDevice);

	~CrGPUStackAllocatorD3D12();

private:

	virtual void InitPS(size_t size) final override;

	virtual void* BeginPS() final override;

	virtual void EndPS() final override;
};
