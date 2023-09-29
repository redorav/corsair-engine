#pragma once

#include "ICrModelDecoder.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class CrModelDecoderUFBX final : public ICrModelDecoder
{
public:

	virtual CrRenderModelHandle Decode(const CrFileHandle& file) override;
};