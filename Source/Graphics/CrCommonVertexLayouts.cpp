#include "Graphics/CrRendering_pch.h"

#include "Graphics/CrCommonVertexLayouts.h"

crgfx::VertexDescriptor SimpleVertexDescriptor
({
	crgfx::VertexAttribute(crgfx::VertexSemantic::Position, crgfx::DataFormat::RGBA16_Float, 0),
	crgfx::VertexAttribute(crgfx::VertexSemantic::Color, crgfx::DataFormat::RGBA8_Unorm, 0),
	crgfx::VertexAttribute(crgfx::VertexSemantic::Normal, crgfx::DataFormat::RGBA8_Unorm, 0),
	crgfx::VertexAttribute(crgfx::VertexSemantic::Tangent, crgfx::DataFormat::RGBA8_Unorm, 0),
	crgfx::VertexAttribute(crgfx::VertexSemantic::TexCoord0, crgfx::DataFormat::RG16_Float, 0)
});

crgfx::VertexDescriptor NullVertexDescriptor({});

crgfx::VertexDescriptor PositionVertexDescriptor
({
	crgfx::VertexAttribute(crgfx::VertexSemantic::Position, crgfx::DataFormat::RGBA16_Float, 0),
});

crgfx::VertexDescriptor AdditionalVertexDescriptor
({
	crgfx::VertexAttribute(crgfx::VertexSemantic::Color, crgfx::DataFormat::RGBA8_Unorm, 0),
	crgfx::VertexAttribute(crgfx::VertexSemantic::Normal, crgfx::DataFormat::RGBA8_Snorm, 0),
	crgfx::VertexAttribute(crgfx::VertexSemantic::Tangent, crgfx::DataFormat::RGBA8_Unorm, 0),
	crgfx::VertexAttribute(crgfx::VertexSemantic::TexCoord0, crgfx::DataFormat::RG16_Float, 0)
});

crgfx::VertexDescriptor ComplexVertexDescriptor
({
	crgfx::VertexAttribute(crgfx::VertexSemantic::Position, crgfx::DataFormat::RGBA16_Float, 0),
	crgfx::VertexAttribute(crgfx::VertexSemantic::Color, crgfx::DataFormat::RGBA8_Unorm, 1),
	crgfx::VertexAttribute(crgfx::VertexSemantic::Normal, crgfx::DataFormat::RGBA8_Unorm, 1),
	crgfx::VertexAttribute(crgfx::VertexSemantic::Tangent, crgfx::DataFormat::RGBA8_Unorm, 1),
	crgfx::VertexAttribute(crgfx::VertexSemantic::TexCoord0, crgfx::DataFormat::RG16_Float, 1)
});