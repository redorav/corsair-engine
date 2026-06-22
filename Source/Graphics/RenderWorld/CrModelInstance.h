#pragma once

#include "Core/CrTypedId.h"

#include "Graphics/CrVisibility.h"

#include "Math/CrHlslppMatrixFloatType.h"

#include "World/CrEntity.h"

#include "crstl/intrusive_ptr.h"

class CrModelInstance;
using CrModelInstanceID = CrTypedID<CrModelInstance, uint32_t>;

class CrModelInstance final : public CrEntity
{
public:

	CrModelInstance() {}

	CrModelInstance(CrModelInstanceID modelInstanceID)
		: m_transform(float4x4::identity())
	{
		m_entityID.type = crntt::EntityType::ModelInstance;
		m_entityID.instanceID = modelInstanceID.id;
	}

	void SetTransform(const float4x4& transform) { m_transform = transform; }
	const float4x4& GetTransform() const { return m_transform; }

	void SetPosition(float3 position) { m_transform[3].xyz = position; }
	float3 GetPosition() const { return m_transform[3].xyz; }

	void SetRenderModel(const CrRenderModelHandle& renderModel) { m_renderModel = renderModel; }
	const CrRenderModelHandle& GetRenderModel() const { return m_renderModel; }

private:

	float4x4 m_transform;

	CrRenderModelHandle m_renderModel;

	CrBoundingBox m_boundingBox;
};
