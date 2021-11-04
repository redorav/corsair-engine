#pragma once

#include "Core/CrAlignment.h"
#include "Rendering/CrStackAllocator.h"

// Manages transient memory allocated per frame for CPU resources
class CrCPUStackAllocator : public CrStackAllocator
{
public:

	~CrCPUStackAllocator();

	void Initialize(size_t size);

	void Reset();
};
