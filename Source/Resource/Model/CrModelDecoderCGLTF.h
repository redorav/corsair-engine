#pragma once

#include "ICrModelDecoder.h"
#include "Core/CrCoreForwardDeclarations.h"

class CrModelDecoderCGLTF final : public ICrModelDecoder
{
public:
	virtual CrRenderModelHandle Decode(const CrFileSharedHandle& file) override;
};