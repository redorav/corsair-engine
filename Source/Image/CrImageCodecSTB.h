#pragma once

#include "ICrImageCodec.h"

class CrImageDecoderSTB final : public ICrImageDecoder
{
public:

	virtual CrImageHandle Decode(const CrFileSharedHandle& file) const override;

	virtual CrImageHandle Decode(void* data, uint64_t dataSize) const override;
};

class CrImageEncoderSTB final : public ICrImageEncoder
{
	virtual void Encode(const CrImageHandle& image, const CrFileSharedHandle& file) const override;

	virtual void Encode(const CrImageHandle& image, void* data, uint64_t dataSize) const override;
};