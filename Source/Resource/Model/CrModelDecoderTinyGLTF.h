#pragma once

#include "ICrModelDecoder.h"
#include "Core/CrCoreForwardDeclarations.h"

class CrModelDecoderTinyGLTF final : public ICrModelDecoder
{
public:
	virtual CrRenderModelSharedHandle Decode(const CrFileSharedHandle& file) override;
};