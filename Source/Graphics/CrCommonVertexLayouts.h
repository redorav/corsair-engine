#pragma once

#include "Graphics/GPUBuffer.h"

struct SimpleVertex
{
	crgfx::VertexElement<half, crgfx::DataFormat::RGBA16_Float> position;
	crgfx::VertexElement<uint8_t, crgfx::DataFormat::RGBA8_Unorm> color;
	crgfx::VertexElement<uint8_t, crgfx::DataFormat::RGBA8_Unorm> normal;
	crgfx::VertexElement<uint8_t, crgfx::DataFormat::RGBA8_Unorm> tangent;
	crgfx::VertexElement<half, crgfx::DataFormat::RG16_Float> uv;
};

extern crgfx::VertexDescriptor SimpleVertexDescriptor;
extern crgfx::VertexDescriptor NullVertexDescriptor;

struct ComplexVertexPosition
{
	crgfx::VertexElement<half, crgfx::DataFormat::RGBA16_Float> position;
};

struct ComplexVertexAdditional
{
	crgfx::VertexElement<uint8_t, crgfx::DataFormat::RGBA8_Unorm> color;
	crgfx::VertexElement<int8_t, crgfx::DataFormat::RGBA8_Snorm> normal;
	crgfx::VertexElement<uint8_t, crgfx::DataFormat::RGBA8_Unorm> tangent;
	crgfx::VertexElement<half, crgfx::DataFormat::RG16_Float> uv;
};

extern crgfx::VertexDescriptor PositionVertexDescriptor;

extern crgfx::VertexDescriptor AdditionalVertexDescriptor;

extern crgfx::VertexDescriptor ComplexVertexDescriptor;