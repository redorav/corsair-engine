#include "Graphics/CrRendering_pch.h"
#include "CrVertexDescriptor.h"

namespace crgfx
{
	namespace VertexSemantic
	{
		Data::Data(VertexSemantic::T semantic, const char* name) : semantic(semantic), semanticName(name)
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

		crstl::array<Data, VertexSemantic::Count> VertexSemanticData;

		bool CreateVertexSemanticData()
		{
			VertexSemanticData[VertexSemantic::Position] = Data(VertexSemantic::Position, "POSITION");
			VertexSemanticData[VertexSemantic::Normal] = Data(VertexSemantic::Normal, "NORMAL");
			VertexSemanticData[VertexSemantic::Tangent] = Data(VertexSemantic::Tangent, "TANGENT");
			VertexSemanticData[VertexSemantic::Color] = Data(VertexSemantic::Color, "COLOR");
			VertexSemanticData[VertexSemantic::TexCoord0] = Data(VertexSemantic::TexCoord0, "TEXCOORD0");

			for (uint32_t s = 0; s < VertexSemanticData.size(); ++s)
			{
				CrAssertMsg(VertexSemanticData[s].semantic != VertexSemantic::Count, "Semantic not initialized");
			}

			return true;
		}

		bool DummyCreateVertexSemanticData = CreateVertexSemanticData();
	};
}