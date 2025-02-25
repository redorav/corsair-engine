#pragma once

#include "ICrImageCodec.h"

namespace ddspp
{
	struct Descriptor;
}

class CrImageDecoderDDS final : public ICrImageDecoder
{
public:

	CrImageDecoderDDS();

	virtual CrImageHandle Decode(crstl::file& file) const override;

	virtual CrImageHandle Decode(void* data, uint64_t dataSize) const override;

private:

	void SetImageProperties(CrImageHandle& image, const ddspp::Descriptor& desc) const;
};

class CrImageEncoderDDS final : public ICrImageEncoder
{
public:

	CrImageEncoderDDS();

	virtual void Encode(const CrImageHandle& image, CrWriteFileStream& fileStream) const override;

	virtual void Encode(const CrImageHandle& image, void* data, uint64_t dataSize) const override;

	virtual bool IsImageFormatSupported(cr3d::DataFormat::T format) const override;

private:

	template<typename StreamT>
	void EncodeInternal(const CrImageHandle& image, StreamT& stream) const;
};