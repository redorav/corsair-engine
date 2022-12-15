#pragma once

#include "ICrImageCodec.h"
#include "Rendering/CrImage.h"

class CrImageDecoderSTB final : public ICrImageDecoder
{
public:

	virtual CrImageHandle Decode(const CrFileHandle& file) const override;

	virtual CrImageHandle Decode(void* data, uint64_t dataSize) const override;
};

class CrImageEncoderSTB final : public ICrImageEncoder
{
public:

	CrImageEncoderSTB(CrImageContainerFormat::T containerFormat);

	virtual void Encode(const CrImageHandle& image, const CrFileHandle& file) const override;

	virtual void Encode(const CrImageHandle& image, void* data, uint64_t dataSize) const override;

	virtual bool IsImageFormatSupported(cr3d::DataFormat::T format) const override;

private:

	template<typename FunctionT>
	void Encode(const CrImageHandle& image, const CrFileHandle& file, const FunctionT& function);
};