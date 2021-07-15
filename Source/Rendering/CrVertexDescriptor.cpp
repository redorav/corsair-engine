#include "CrRendering_pch.h"
#include "CrVertexDescriptor.h"

namespace CrVertexSemantic
{
	CrArray<CrVertexSemanticData, CrVertexSemantic::Count> VertexSemanticData;

	bool CreateVertexSemanticData()
	{
		VertexSemanticData[CrVertexSemantic::Position]  = CrVertexSemanticData(CrVertexSemantic::Position, "POSITION");
		VertexSemanticData[CrVertexSemantic::Normal]    = CrVertexSemanticData(CrVertexSemantic::Normal, "NORMAL");
		VertexSemanticData[CrVertexSemantic::Tangent]   = CrVertexSemanticData(CrVertexSemantic::Tangent, "TANGENT");
		VertexSemanticData[CrVertexSemantic::Color]     = CrVertexSemanticData(CrVertexSemantic::Color, "COLOR");
		VertexSemanticData[CrVertexSemantic::TexCoord0] = CrVertexSemanticData(CrVertexSemantic::TexCoord0, "TEXCOORD0");

		for (uint32_t s = 0; s < VertexSemanticData.size(); ++s)
		{
			CrAssertMsg(VertexSemanticData[s].semantic != CrVertexSemantic::Count, "Semantic not initialized");
		}

		return true;
	}

	bool DummyCreateVertexSemanticData = CreateVertexSemanticData();
};