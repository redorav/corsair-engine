#pragma once

#include "ICrImageDecoder.h"

class CrImageDecoderSTB final : public ICrImageDecoder
{
public:

	virtual CrImageHandle Decode(const CrFileSharedHandle& file) override;
	virtual CrImageHandle Decode(void* data, uint64_t dataSize) override;
};