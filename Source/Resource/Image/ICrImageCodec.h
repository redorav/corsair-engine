#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Core/CrCoreForwardDeclarations.h"
#include "Resource/Image/CrImageForwardDeclarations.h"

#include <stdint.h>

class ICrImageCodec
{
	CrImageContainerFormat::T GetContainerFormat() const { return m_containerFormat; }

protected:

	CrImageContainerFormat::T m_containerFormat;
};

class ICrImageDecoder : public ICrImageCodec
{
public:

	virtual ~ICrImageDecoder() {}

	// Decode image in file
	virtual CrImageHandle Decode(const CrFileSharedHandle& file) const = 0;

	// Decode image in provided data
	virtual CrImageHandle Decode(void* data, uint64_t dataSize) const = 0;
};

class ICrImageEncoder : public ICrImageCodec
{
public:

	virtual ~ICrImageEncoder() {}

	// Encode image to file
	virtual void Encode(const CrImageHandle& image, const CrFileSharedHandle& file) const = 0;

	// Encode image in raw data
	virtual void Encode(const CrImageHandle& image, void* data, uint64_t dataSize) const = 0;

	// Query whether this encoder can encode this data format
	virtual bool IsImageFormatSupported(cr3d::DataFormat::T format) const = 0;
};