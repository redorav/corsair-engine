#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrImage.h"
#include "Rendering/CrRendering.h"

CrImageDescriptor::CrImageDescriptor()
	: width(1)
	, height(1)
	, depth(1)
	, mipmapCount(1)
	, data(nullptr)
	, dataSize(0)
	, format(cr3d::DataFormat::RGBA8_Unorm)
	, type(cr3d::TextureType::Tex2D)
{

}

CrImage::CrImage() : m_width(1), m_height(1), m_depth(1), m_format(cr3d::DataFormat::RGBA8_Unorm)
{

}

CrImage::CrImage(const CrImageDescriptor& descriptor)
	: m_width(descriptor.width)
	, m_height(descriptor.height)
	, m_depth(descriptor.depth)
	, m_mipmapCount(descriptor.mipmapCount)
	, m_format(descriptor.format)
	, m_type(descriptor.type)
{
	m_data.resize(descriptor.dataSize);
	memcpy(m_data.data(), descriptor.data, m_data.size());
}

CrImage::~CrImage()
{

}