#pragma once

#include "Core/CrAlignment.h"
#include "Core/SmartPointers/CrIntrusivePtr.h"

#include "Rendering/CrStackAllocator.h"

// Manages transient memory allocated per frame for CPU resources
class CrCPUStackAllocator : public CrStackAllocator, public CrIntrusivePtrInterface
{
public:

	~CrCPUStackAllocator();

	void Initialize(size_t size);

	void Reset();
};