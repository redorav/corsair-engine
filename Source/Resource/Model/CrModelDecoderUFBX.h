#pragma once

#include "ICrModelDecoder.h"

#include "Graphics/CrGraphicsForwardDeclarations.h"

class CrModelDecoderUFBX final : public ICrModelDecoder
{
public:

	virtual CrRenderModelHandle Decode(const crstl::file& file) override;
};