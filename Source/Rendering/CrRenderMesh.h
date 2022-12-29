#pragma once

#include "Core/SmartPointers/CrIntrusivePtr.h"
#include "Core/Containers/CrVector.h"

#include "Rendering/CrVisibility.h"
#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrVertexDescriptor.h"

class CrRenderMesh final : public CrIntrusivePtrInterface
{
public:

	~CrRenderMesh();

	const CrVertexDescriptor& GetVertexDescriptor() const { return m_vertexDescriptor; }

	void AddVertexBuffer(const CrVertexBufferHandle& vertexBuffer);

	const CrVertexBufferHandle& GetVertexBuffer(uint32_t index) const { return m_vertexBuffers[index]; }

	uint32_t GetVertexBufferCount() const { return (uint32_t)m_vertexBuffers.size(); }

	const CrBoundingBox& GetBoundingBox() const { return m_boundingBox; }

	void SetBoundingBox(const CrBoundingBox& boundingBox) { m_boundingBox = boundingBox; }
	
	const CrIndexBufferHandle& GetIndexBuffer() const { return m_indexBuffer; }

	void SetIndexBuffer(const CrIndexBufferHandle& indexBuffer);

private:

	void MergeVertexDescriptors();

	CrVector<CrVertexBufferHandle> m_vertexBuffers;

	CrVertexDescriptor m_vertexDescriptor;

	CrIndexBufferHandle m_indexBuffer;

	CrBoundingBox m_boundingBox;
};