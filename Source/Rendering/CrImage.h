#pragma once

#include "Core/Containers/CrVector.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class CrImage
{
public:

	CrImage();

	uint32_t GetWidth();

	uint32_t GetHeight();

	uint32_t GetDepth();

	cr3d::DataFormat::T GetFormat();

	const void* GetData();

	uint32_t GetDataSize();

	// TODO need proper destruction
	~CrImage();

public: // TODO remove

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_depth;
	uint32_t m_numMipmaps;

	CrVector<unsigned char> m_data;
	unsigned char* m_dataPointer;
	uint32_t m_dataSize;
	cr3d::DataFormat::T m_format;
	cr3d::TextureType m_type;
};