#pragma once

#include "Graphics/CrGPUBuffer.h"

struct SimpleVertex
{
	CrVertexElement<half, crgfx::DataFormat::RGBA16_Float> position;
	CrVertexElement<uint8_t, crgfx::DataFormat::RGBA8_Unorm> color;
	CrVertexElement<uint8_t, crgfx::DataFormat::RGBA8_Unorm> normal;
	CrVertexElement<uint8_t, crgfx::DataFormat::RGBA8_Unorm> tangent;
	CrVertexElement<half, crgfx::DataFormat::RG16_Float> uv;
};

extern CrVertexDescriptor SimpleVertexDescriptor;
extern CrVertexDescriptor NullVertexDescriptor;

struct ComplexVertexPosition
{
	CrVertexElement<half, crgfx::DataFormat::RGBA16_Float> position;
};

struct ComplexVertexAdditional
{
	CrVertexElement<uint8_t, crgfx::DataFormat::RGBA8_Unorm> color;
	CrVertexElement<int8_t, crgfx::DataFormat::RGBA8_Snorm> normal;
	CrVertexElement<uint8_t, crgfx::DataFormat::RGBA8_Unorm> tangent;
	CrVertexElement<half, crgfx::DataFormat::RG16_Float> uv;
};

extern CrVertexDescriptor PositionVertexDescriptor;

extern CrVertexDescriptor AdditionalVertexDescriptor;

extern CrVertexDescriptor ComplexVertexDescriptor;