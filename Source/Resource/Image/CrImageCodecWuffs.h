#pragma once

#include "ICrImageCodec.h"
#include "Rendering/CrImage.h"

class CrImageDecoderWuffs final : public ICrImageDecoder
{
public:

	virtual CrImageHandle Decode(crstl::file& file) const override;

	virtual CrImageHandle Decode(void* data, uint64_t dataSize) const override;
};

class CrImageEncoderWuffs final : public ICrImageEncoder
{
public:

	CrImageEncoderWuffs(CrImageContainerFormat::T containerFormat);

	virtual void Encode(const CrImageHandle& image, CrWriteFileStream& fileStream) const override;

	virtual void Encode(const CrImageHandle& image, void* data, uint64_t dataSize) const override;

	virtual bool IsImageFormatSupported(cr3d::DataFormat::T format) const override;
};