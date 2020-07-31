#pragma once

#include "ICrImageDecoder.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class CrImageDecoderDDS final : public ICrImageDecoder
{
public:

	virtual CrImageHandle Decode(const CrFileSharedHandle& file) override;
};