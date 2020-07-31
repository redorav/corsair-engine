#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Image/CrImageForwardDeclarations.h"

class ICrImageDecoder
{
public:

	virtual CrImageHandle Decode(const CrFileSharedHandle& file) = 0;
};