#pragma once

#include "Core/CrTypedId.h"

class CrRenderModelInstance;
using CrModelInstanceId = CrTypedId<CrRenderModelInstance, uint32_t>;

class CrRenderModelInstance
{
public:

	CrRenderModelInstance() : m_instanceId(CrModelInstanceId::MaxId) {}

	CrRenderModelInstance(CrModelInstanceId instanceId)
		: m_instanceId(instanceId) {}

	CrModelInstanceId GetId() const { return m_instanceId; }

	void SetVisibilityId(uint32_t visibilityId) { m_visibilityId = visibilityId; }

	void RemoveFromWorld();

private:

	CrModelInstanceId m_instanceId;

	uint32_t m_visibilityId;
};