#include "Graphics/CrRendering_pch.h"

#include "Graphics/GPUBuffer.h"
#include "CrRenderMesh.h"

CrRenderMesh::~CrRenderMesh()
{

}

void CrRenderMesh::AddVertexBuffer(const crgfx::VertexBufferHandle& vertexBuffer)
{
	m_vertexBuffers.push_back(vertexBuffer);

	MergeVertexDescriptors();
}

void CrRenderMesh::SetIndexBuffer(const crgfx::IndexBufferHandle& indexBuffer)
{
	m_indexBuffer = indexBuffer;
}

// TODO Figure out best way of passing multiple references
//void CrMesh::AddVertexBuffers(std::initializer_list<const CrVertexBufferSharedHandle&> vertexBuffers)
//{
//	//auto it = vertexBuffers.begin();
//	//while (it != vertexBuffers.end())
//	//{
//	//	m_vertexBuffers.push_back(*it);
//	//	++it;
//	//}
//	//
//	//MergeVertexDescriptors();
//}

void CrRenderMesh::MergeVertexDescriptors()
{
	m_vertexDescriptor = crgfx::VertexDescriptor();

	for (uint32_t streamId = 0; streamId < m_vertexBuffers.size(); ++streamId)
	{
		const crgfx::VertexDescriptor& vertexBufferDescriptor = m_vertexBuffers[streamId]->GetVertexDescriptor();

		for (uint32_t i = 0; i < vertexBufferDescriptor.GetAttributeCount(); ++i)
		{
			crgfx::VertexAttribute attribute = vertexBufferDescriptor.GetAttribute(i);
			attribute.streamId = streamId;
			m_vertexDescriptor.AddAttribute(attribute);
		}
	}
}