#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "CrResourceManager.h"

class ICrModelDecoder
{
public:
	virtual CrRenderModelSharedHandle Decode(const CrFileSharedHandle& file) = 0;
};