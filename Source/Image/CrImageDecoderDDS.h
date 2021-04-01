#pragma once

#include "ICrImageDecoder.h"

namespace ddspp
{
	struct Descriptor;
}

class CrImageDecoderDDS final : public ICrImageDecoder
{
public:

	virtual CrImageHandle Decode(const CrFileSharedHandle& file) override;
	virtual CrImageHandle Decode(void* data, uint64_t dataSize) override;

private:

	void SetImageProperties(CrImageHandle& image, const ddspp::Descriptor* desc);
};