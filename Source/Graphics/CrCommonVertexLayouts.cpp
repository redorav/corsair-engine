#include "Graphics/CrRendering_pch.h"

#include "Graphics/CrCommonVertexLayouts.h"

CrVertexDescriptor SimpleVertexDescriptor
({
	CrVertexAttribute(CrVertexSemantic::Position, crgfx::DataFormat::RGBA16_Float, 0),
	CrVertexAttribute(CrVertexSemantic::Color, crgfx::DataFormat::RGBA8_Unorm, 0),
	CrVertexAttribute(CrVertexSemantic::Normal, crgfx::DataFormat::RGBA8_Unorm, 0),
	CrVertexAttribute(CrVertexSemantic::Tangent, crgfx::DataFormat::RGBA8_Unorm, 0),
	CrVertexAttribute(CrVertexSemantic::TexCoord0, crgfx::DataFormat::RG16_Float, 0)
});

CrVertexDescriptor NullVertexDescriptor({});

CrVertexDescriptor PositionVertexDescriptor
({
	CrVertexAttribute(CrVertexSemantic::Position, crgfx::DataFormat::RGBA16_Float, 0),
});

CrVertexDescriptor AdditionalVertexDescriptor
({
	CrVertexAttribute(CrVertexSemantic::Color, crgfx::DataFormat::RGBA8_Unorm, 0),
	CrVertexAttribute(CrVertexSemantic::Normal, crgfx::DataFormat::RGBA8_Snorm, 0),
	CrVertexAttribute(CrVertexSemantic::Tangent, crgfx::DataFormat::RGBA8_Unorm, 0),
	CrVertexAttribute(CrVertexSemantic::TexCoord0, crgfx::DataFormat::RG16_Float, 0)
});

CrVertexDescriptor ComplexVertexDescriptor
({
	CrVertexAttribute(CrVertexSemantic::Position, crgfx::DataFormat::RGBA16_Float, 0),
	CrVertexAttribute(CrVertexSemantic::Color, crgfx::DataFormat::RGBA8_Unorm, 1),
	CrVertexAttribute(CrVertexSemantic::Normal, crgfx::DataFormat::RGBA8_Unorm, 1),
	CrVertexAttribute(CrVertexSemantic::Tangent, crgfx::DataFormat::RGBA8_Unorm, 1),
	CrVertexAttribute(CrVertexSemantic::TexCoord0, crgfx::DataFormat::RG16_Float, 1)
});