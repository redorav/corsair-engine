#pragma once

#include "Graphics/CrVisibility.h"
#include "Graphics/CrGraphicsForwardDeclarations.h"
#include "Graphics/VertexDescriptor.h"

#include "crstl/intrusive_ptr.h"
#include "crstl/vector.h"

class CrRenderMesh final : public crstl::intrusive_ptr_interface_delete
{
public:

	~CrRenderMesh();

	const crgfx::VertexDescriptor& GetVertexDescriptor() const { return m_vertexDescriptor; }

	void AddVertexBuffer(const crgfx::VertexBufferHandle& vertexBuffer);

	const crgfx::VertexBufferHandle& GetVertexBuffer(uint32_t index) const { return m_vertexBuffers[index]; }

	uint32_t GetVertexBufferCount() const { return (uint32_t)m_vertexBuffers.size(); }

	const CrBoundingBox& GetBoundingBox() const { return m_boundingBox; }

	void SetBoundingBox(const CrBoundingBox& boundingBox) { m_boundingBox = boundingBox; }
	
	const crgfx::IndexBufferHandle& GetIndexBuffer() const { return m_indexBuffer; }

	void SetIndexBuffer(const crgfx::IndexBufferHandle& indexBuffer);

	void SetIsDoubleSided(bool isDoubleSided) { m_isDoubleSided = isDoubleSided; }

	bool GetIsDoubleSided() const { return m_isDoubleSided; }

private:

	void MergeVertexDescriptors();

	bool m_isDoubleSided = false;

	crstl::vector<crgfx::VertexBufferHandle> m_vertexBuffers;

	crgfx::VertexDescriptor m_vertexDescriptor;

	crgfx::IndexBufferHandle m_indexBuffer;

	CrBoundingBox m_boundingBox;
};