#pragma once

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/Containers/CrVector.h"

#include "Rendering/CrVisibility.h"
#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrVertexDescriptor.h"

class CrMesh
{
public:

	const CrVertexDescriptor& GetVertexDescriptor() const { return m_vertexDescriptor; }

	void AddVertexBuffer(const CrVertexBufferSharedHandle& vertexBuffer);

	void AddVertexBuffers(std::initializer_list<const CrVertexBufferSharedHandle&> vertexBuffers);

	const CrVertexBufferSharedHandle& GetVertexBuffer(uint32_t index) const { return m_vertexBuffers[index]; }

	uint32_t GetVertexBufferCount() const { return (uint32_t)m_vertexBuffers.size(); }

	const CrBoundingBox& GetBoundingBox() const { return m_boundingBox; }

	void SetBoundingBox(const CrBoundingBox& boundingBox) { m_boundingBox = boundingBox; }
	
	const CrIndexBufferSharedHandle& GetIndexBuffer() const { return m_indexBuffer; }

	void SetIndexBuffer(const CrIndexBufferSharedHandle& indexBuffer) { m_indexBuffer = indexBuffer; }

	CrVertexDescriptor MergeVertexDescriptors(const CrVertexDescriptor& vertexDescriptorA, const CrVertexDescriptor& vertexDescriptorB);

private:

	void MergeVertexDescriptors();

	CrVector<CrVertexBufferSharedHandle> m_vertexBuffers;

	CrVertexDescriptor m_vertexDescriptor;

	CrIndexBufferSharedHandle m_indexBuffer;

	CrBoundingBox m_boundingBox;
};

using CrRenderMeshSharedHandle = CrSharedPtr<CrMesh>;