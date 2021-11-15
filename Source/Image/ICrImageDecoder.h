#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Image/CrImageForwardDeclarations.h"

#include <stdint.h>

class ICrImageDecoder
{
public:

	virtual ~ICrImageDecoder() {}

	virtual CrImageHandle Decode(const CrFileSharedHandle& file) = 0;
	virtual CrImageHandle Decode(void* data, uint64_t dataSize) = 0;
};