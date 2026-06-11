#pragma once

#include "Core/CrAlignment.h"

#include "Rendering/CrStackAllocator.h"

#include "crstl/intrusive_ptr.h"

// Manages transient memory allocated per frame for CPU resources
class CrCPUStackAllocator final : public CrStackAllocator, public crstl::intrusive_ptr_interface_delete
{
public:

	~CrCPUStackAllocator();

	void Initialize(size_t size);

	void Reset();
};