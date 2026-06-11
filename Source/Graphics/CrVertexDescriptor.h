#pragma once

#include "Core/CrHash.h"

#include "crstl/array.h"
#include "crstl/bitset.h"
#include "crstl/string.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRendering.h"
#include "Rendering/CrDataFormats.h"

#include "Math/CrVectorT.h"

// These are the vertex semantics that we can use. They need to mirror the vertex semantics
// in the shader
// TODO It would be good to have a shared file that has the names
namespace CrVertexSemantic
{
	enum T : uint32_t
	{
		Position,
		Normal,
		Tangent,
		Color,
		TexCoord0,
		Count
	};

	struct Data
	{
		Data() {}
		Data(CrVertexSemantic::T semantic, const char* name);

		CrVertexSemantic::T semantic;
		crstl::string semanticName;
		
		uint32_t index; // Which index this semantic has
		uint32_t indexOffset; // Where in the string the first digit is
	};

	extern crstl::array<Data, CrVertexSemantic::Count> VertexSemanticData;

	bool CreateVertexSemanticData();

	inline const Data& GetData(T semantic)
	{
		return VertexSemanticData[semantic];
	}

	inline const char* ToString(T semantic)
	{
		return VertexSemanticData[semantic].semanticName.c_str();
	}

	inline CrVertexSemantic::T FromString(const char* semanticString)
	{
		if (semanticString)
		{
			for (uint32_t s = 0; s < VertexSemanticData.size(); ++s)
			{
				if (VertexSemanticData[s].semanticName == semanticString)
				{
					return (CrVertexSemantic::T)s;
				}
			}
		}

		return CrVertexSemantic::Count;
	}
};

template<typename T, cr3d::DataFormat::T F>
class CrVertexElement
{
public:

	CrVertexElement()
	{
		StaticAsserts();
	}

	CrVertexElement(std::initializer_list<T> l) : data(l)
	{
		StaticAsserts();
	}

	static cr3d::DataFormat::T GetFormat()
	{
		return F;
	}

	uint32_t GetDataSize()
	{
		return sizeof(data);
	}

	VectorT<T, cr3d::DataFormats[F].numComponents> data;

private:

	void StaticAsserts()
	{
		static_assert(cr3d::CrTypeName<T>() == cr3d::DataFormats[F].name, "Type does not match");
		static_assert(F == cr3d::DataFormats[F].format, "Vertex format is in an incorrect position with respect to the enum"); // Do not compile if the order in the array does not match the order in the enum
		static_assert(sizeof(T) == cr3d::DataFormats[F].elementSizeR / 8, "Data type size does not match");
	}
};

struct CrVertexAttribute
{
	CrVertexAttribute() : semantic(0), format(0), streamId(0) {}

	CrVertexAttribute(CrVertexSemantic::T semantic, cr3d::DataFormat::T format, uint32_t streamId)
		: semantic((uint16_t)semantic), format((uint16_t)format), streamId((uint16_t)streamId) {}

	uint16_t semantic : 4; // CrVertexSemantic::T
	uint16_t format   : 6; // cr3d::DataFormat::T - we only care for the uncompressed formats
	uint16_t streamId : 3;
};

static_assert(sizeof(CrVertexAttribute) == 2, "Vertex attribute size mismatch");
static_assert(cr3d::DataFormat::LastUncompressed < 64, "Formats out of range");

// A vertex format defines the layout for a vertex buffer or a mesh
// If it is owned by a vertex buffer, streamId is always 0
// If it is owned by a mesh, stream id belongs to the stream we've
// decided to put it in. The input layout of a pipeline will need
// to mirror this layout exactly when instantiated and the mesh
// will have to bind the necessary data
struct CrVertexDescriptor
{
	CrVertexDescriptor() {}

	CrVertexDescriptor(const std::initializer_list<CrVertexAttribute>& vertexAttributes)
	{
		auto it = vertexAttributes.begin();
		while (it != vertexAttributes.end())
		{
			AddAttribute(*it);
			++it;
		}
	}

	void AddAttribute(const CrVertexAttribute& attribute)
	{
		m_dataSize += (uint16_t)cr3d::DataFormats[attribute.format].dataOrBlockSize;
		m_streamMask.set(attribute.streamId);
		m_attributes[m_attributeCount] = attribute;
		m_attributeCount++;
	}

	const CrVertexAttribute& GetAttribute(uint32_t attributeIndex) const
	{
		//CrAssertMsg(i < m_attributeCount, "Index out of bounds");
		return m_attributes[attributeIndex];
	}

	uint32_t GetAttributeCount() const
	{
		return m_attributeCount;
	}

	const cr3d::DataFormatInfo& GetDataFormatInfo(uint32_t attributeIndex)
	{
		return cr3d::DataFormats[GetAttribute(attributeIndex).format];
	}

	uint32_t GetStreamCount() const
	{
		return (uint32_t)m_streamMask.count();
	}

	uint32_t GetStreamStride(uint32_t streamId) const
	{
		uint32_t stride = 0;
		for (uint32_t i = 0; i < m_attributeCount; ++i)
		{
			if (m_attributes[i].streamId == streamId)
			{
				stride += cr3d::DataFormats[m_attributes[i].format].dataOrBlockSize;
			}
		}

		return stride;
	}

	cr3d::VertexInputRate GetInputRate(uint32_t streamId) const
	{
		return (cr3d::VertexInputRate)m_inputRates[streamId];
	}

	uint32_t GetDataSize() const
	{
		return m_dataSize;
	}

	CrHash ComputeHash() const
	{
		return CrHash(*this);
	}

private:

	// Attributes
	crstl::array<CrVertexAttribute, cr3d::MaxVertexAttributes> m_attributes;

	// Indicates which streams are being used
	crstl::bitset<cr3d::MaxVertexStreams, uint16_t> m_streamMask;
	
	// Indicate which input rates are used (0 - vertex, 1 - instance)
	crstl::bitset<cr3d::MaxVertexStreams, uint16_t> m_inputRates;

	// Total number of attributes
	uint16_t m_attributeCount = 0;

	// Total data size of this vertex
	uint16_t m_dataSize = 0;
};