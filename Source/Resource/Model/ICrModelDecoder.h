#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Resource/CrResourceManager.h"

class ICrModelDecoder
{
public:

	virtual ~ICrModelDecoder() {}

	virtual CrRenderModelSharedHandle Decode(const CrFileSharedHandle& file) = 0;
};