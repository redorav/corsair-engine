#pragma once

#include "ICrImageDecoder.h"

namespace ddspp
{
	struct Descriptor;
}

class CrImageDecoderDDS final : public ICrImageDecoder
{
public:

	virtual CrImageHandle Decode(const CrFileSharedHandle& file) const override;

	virtual CrImageHandle Decode(void* data, uint64_t dataSize) const override;

private:

	void SetImageProperties(CrImageHandle& image, const ddspp::Descriptor& desc) const;
};

class CrImageEncoderDDS final : public ICrImageEncoder
{
public:

	virtual void Encode(const CrImageHandle& image, const CrFileSharedHandle& file) const override;

	virtual void Encode(const CrImageHandle& image, void* data, uint64_t dataSize) const override;

private:

	template<typename StreamT>
	void Encode(const CrImageHandle& image, StreamT& stream) const;
};