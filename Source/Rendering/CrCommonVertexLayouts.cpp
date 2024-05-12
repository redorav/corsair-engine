#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrCommonVertexLayouts.h"

CrVertexDescriptor SimpleVertexDescriptor
({
	CrVertexAttribute(CrVertexSemantic::Position, cr3d::DataFormat::RGBA16_Float, 0),
	CrVertexAttribute(CrVertexSemantic::Color, cr3d::DataFormat::RGBA8_Unorm, 0),
	CrVertexAttribute(CrVertexSemantic::Normal, cr3d::DataFormat::RGBA8_Unorm, 0),
	CrVertexAttribute(CrVertexSemantic::Tangent, cr3d::DataFormat::RGBA8_Unorm, 0),
	CrVertexAttribute(CrVertexSemantic::TexCoord0, cr3d::DataFormat::RG16_Float, 0)
});

CrVertexDescriptor NullVertexDescriptor({});

CrVertexDescriptor PositionVertexDescriptor
({
	CrVertexAttribute(CrVertexSemantic::Position, cr3d::DataFormat::RGBA16_Float, 0),
});

CrVertexDescriptor AdditionalVertexDescriptor
({
	CrVertexAttribute(CrVertexSemantic::Color, cr3d::DataFormat::RGBA8_Unorm, 0),
	CrVertexAttribute(CrVertexSemantic::Normal, cr3d::DataFormat::RGBA8_Snorm, 0),
	CrVertexAttribute(CrVertexSemantic::Tangent, cr3d::DataFormat::RGBA8_Unorm, 0),
	CrVertexAttribute(CrVertexSemantic::TexCoord0, cr3d::DataFormat::RG16_Float, 0)
});

CrVertexDescriptor ComplexVertexDescriptor
({
	CrVertexAttribute(CrVertexSemantic::Position, cr3d::DataFormat::RGBA16_Float, 0),
	CrVertexAttribute(CrVertexSemantic::Color, cr3d::DataFormat::RGBA8_Unorm, 1),
	CrVertexAttribute(CrVertexSemantic::Normal, cr3d::DataFormat::RGBA8_Unorm, 1),
	CrVertexAttribute(CrVertexSemantic::Tangent, cr3d::DataFormat::RGBA8_Unorm, 1),
	CrVertexAttribute(CrVertexSemantic::TexCoord0, cr3d::DataFormat::RG16_Float, 1)
});