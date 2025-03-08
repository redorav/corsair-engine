#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "crstl/intrusive_ptr.h"
#include "crstl/vector.h"

namespace CrImageContainerFormat
{
	enum T : uint32_t
	{
		DDS,
		PNG,
		JPG,
		TGA,
		BMP,
		HDR,
		Count,
		None,
	};
};

struct CrImageDescriptor
{
	CrImageDescriptor();

	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t mipmapCount;

	uint8_t* data;
	uint64_t dataSize;

	cr3d::DataFormat::T format;
	cr3d::TextureType type;
};

class CrImage final : public crstl::intrusive_ptr_interface_delete
{
public:

	CrImage();

	CrImage(const CrImageDescriptor& descriptor);

	uint32_t GetWidth() const { return m_width; }

	uint32_t GetHeight() const { return m_height; }

	uint32_t GetDepth() const { return m_depth; }

	cr3d::DataFormat::T GetFormat() const { return m_format; }

	cr3d::TextureType GetType() const { return m_type; }

	const uint8_t* GetData() const { return m_data.data(); }

	uint64_t GetDataSize() const { return m_data.size(); }

	uint32_t GetMipmapCount() const { return m_mipmapCount; }

	~CrImage();

public: // TODO remove

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_depth;
	uint32_t m_mipmapCount;

	crstl::vector<uint8_t> m_data;
	cr3d::DataFormat::T m_format;
	cr3d::TextureType m_type;
};