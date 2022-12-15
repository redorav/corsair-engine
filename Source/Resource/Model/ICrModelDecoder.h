#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Resource/CrResourceManager.h"

class ICrModelDecoder
{
public:

	virtual ~ICrModelDecoder() {}

	virtual CrRenderModelHandle Decode(const CrFileSharedHandle& file) = 0;
};