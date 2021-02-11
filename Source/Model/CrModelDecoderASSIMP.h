#pragma once

#include "ICrModelDecoder.h"

class CrModelDecoderASSIMP final : public ICrModelDecoder
{
public:

	virtual CrRenderModelSharedHandle Decode(const CrFileSharedHandle& file) override;
};