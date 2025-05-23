#pragma once

#include "Rendering/CrVisibility.h"
#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrVertexDescriptor.h"

#include "crstl/intrusive_ptr.h"
#include "crstl/vector.h"

class CrRenderMesh final : public crstl::intrusive_ptr_interface_delete
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

	void SetIsDoubleSided(bool isDoubleSided) { m_isDoubleSided = isDoubleSided; }

	bool GetIsDoubleSided() const { return m_isDoubleSided; }

private:

	void MergeVertexDescriptors();

	bool m_isDoubleSided = false;

	crstl::vector<CrVertexBufferHandle> m_vertexBuffers;

	CrVertexDescriptor m_vertexDescriptor;

	CrIndexBufferHandle m_indexBuffer;

	CrBoundingBox m_boundingBox;
};