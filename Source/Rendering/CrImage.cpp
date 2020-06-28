#include "CrRendering_pch.h"
#include "CrImage.h"

CrImage::CrImage() : m_width(1), m_height(1), m_depth(1), m_data(nullptr), m_format(cr3d::DataFormat::RGBA8_Unorm)
{

}

uint32_t CrImage::GetWidth()
{
	return m_width;
}

uint32_t CrImage::GetHeight()
{
	return m_height;
}

uint32_t CrImage::GetDepth()
{
	return m_depth;
}

cr3d::DataFormat::T CrImage::GetFormat()
{
	return m_format;
}

const void* CrImage::GetData()
{
	return m_dataPointer;
}

uint32_t CrImage::GetDataSize()
{
	return m_dataSize;
}

CrImage::~CrImage()
{

}
