#pragma once

#include "Core/CrTypedId.h"

#include "Rendering/CrVisibility.h"

#include "Math/CrHlslppMatrixFloatType.h"

#include "crstl/intrusive_ptr.h"

class CrModelInstance;
using CrModelInstanceId = CrTypedId<CrModelInstance, uint32_t>;

class CrModelInstance
{
public:

	CrModelInstance() 
		: m_instanceId(CrModelInstanceId::MaxId)
		, m_visibilityId(0xffffffff)
		, m_transform(float4x4::identity())
	{}

	explicit CrModelInstance(CrModelInstanceId instanceId)
		: m_instanceId(instanceId)
		, m_visibilityId(0xffffffff)
		, m_transform(float4x4::identity())
	{}

	void SetTransform(const float4x4& transform) { m_transform = transform; }
	const float4x4& GetTransform() const { return m_transform; }

	void SetPosition(float3 position) { m_transform[3].xyz = position; }
	float3 GetPosition() const { return m_transform[3].xyz; }

	void SetRenderModel(const CrRenderModelHandle& renderModel) { m_renderModel = renderModel; }
	const CrRenderModelHandle& GetRenderModel() const { return m_renderModel; }

	CrModelInstanceId GetId() const { return m_instanceId; }

	void SetVisibilityId(uint32_t visibilityId) { m_visibilityId = visibilityId; }

private:

	float4x4 m_transform;

	CrRenderModelHandle m_renderModel;

	CrBoundingBox m_boundingBox;

	CrModelInstanceId m_instanceId;

	uint32_t m_visibilityId;
};
