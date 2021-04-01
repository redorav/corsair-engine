#pragma once

#include "ICrModelDecoder.h"
#include "Core/CrCoreForwardDeclarations.h"

class CrModelDecoderGLTF final : public ICrModelDecoder
{
public:
	virtual CrRenderModelSharedHandle Decode(const CrFileSharedHandle& file) override;
};