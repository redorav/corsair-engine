#include "CrRendering_pch.h"

#include "Rendering/CrGPUBuffer.h"
#include "CrMesh.h"

void CrMesh::AddVertexBuffer(const CrVertexBufferSharedHandle& vertexBuffer)
{
	m_vertexBuffers.push_back(vertexBuffer);

	MergeVertexDescriptors();
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

void CrMesh::MergeVertexDescriptors()
{
	m_vertexDescriptor = CrVertexDescriptor();

	for (uint32_t streamId = 0; streamId < m_vertexBuffers.size(); ++streamId)
	{
		const CrVertexDescriptor& vertexBufferDescriptor = m_vertexBuffers[streamId]->GetVertexDescriptor();

		for (uint32_t i = 0; i < vertexBufferDescriptor.GetAttributeCount(); ++i)
		{
			CrVertexAttribute attribute = vertexBufferDescriptor.GetAttribute(i);
			attribute.streamId = streamId;
			m_vertexDescriptor.AddAttribute(attribute);
		}
	}
}