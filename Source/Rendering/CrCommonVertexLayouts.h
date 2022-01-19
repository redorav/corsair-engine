#pragma once

#include "Rendering/CrGPUBuffer.h"

struct SimpleVertex
{
	CrVertexElement<half, cr3d::DataFormat::RGBA16_Float> position;
	CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> color;
	CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> normal;
	CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> tangent;
	CrVertexElement<half, cr3d::DataFormat::RG16_Float> uv;
};

extern CrVertexDescriptor SimpleVertexDescriptor;
extern CrVertexDescriptor NullVertexDescriptor;

struct ComplexVertexPosition
{
	CrVertexElement<half, cr3d::DataFormat::RGBA16_Float> position;
};

struct ComplexVertexAdditional
{
	CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> color;
	CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> normal;
	CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> tangent;
	CrVertexElement<half, cr3d::DataFormat::RG16_Float> uv;
};

extern CrVertexDescriptor PositionVertexDescriptor;

extern CrVertexDescriptor AdditionalVertexDescriptor;

extern CrVertexDescriptor ComplexVertexDescriptor;