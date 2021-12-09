#include "CrRendering_pch.h"
#include "CrVertexDescriptor.h"

#include "stdlib.h"

namespace CrVertexSemantic
{
	Data::Data(CrVertexSemantic::T semantic, const char* name) : semantic(semantic), semanticName(name)
	{
		indexOffset = (uint32_t)semanticName.find_first_of("0123456789");
		if (indexOffset != 0xffffffff)
		{
			index = atoi(semanticName.substr(indexOffset, semanticName.length()).c_str());
		}
		else
		{
			index = 0;
		}
	}

	CrArray<Data, CrVertexSemantic::Count> VertexSemanticData;

	bool CreateVertexSemanticData()
	{
		VertexSemanticData[CrVertexSemantic::Position]  = Data(CrVertexSemantic::Position, "POSITION");
		VertexSemanticData[CrVertexSemantic::Normal]    = Data(CrVertexSemantic::Normal, "NORMAL");
		VertexSemanticData[CrVertexSemantic::Tangent]   = Data(CrVertexSemantic::Tangent, "TANGENT");
		VertexSemanticData[CrVertexSemantic::Color]     = Data(CrVertexSemantic::Color, "COLOR");
		VertexSemanticData[CrVertexSemantic::TexCoord0] = Data(CrVertexSemantic::TexCoord0, "TEXCOORD0");

		for (uint32_t s = 0; s < VertexSemanticData.size(); ++s)
		{
			CrAssertMsg(VertexSemanticData[s].semantic != CrVertexSemantic::Count, "Semantic not initialized");
		}

		return true;
	}

	bool DummyCreateVertexSemanticData = CreateVertexSemanticData();
};