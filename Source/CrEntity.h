#pragma once

#include "Core/Containers/CrVector.h"
#include "Core/String/CrString.h"
#include "Math/CrMath.h"

class CrTransform;

class CrEntity
{
public:

	bool isActive = true;

	// TODO Put in transform
	float3 m_position;
	float3 m_scale;
	quaternion m_qrotation;

	CrVector<CrEntity*> gameObjects;

	CrEntity();
	CrEntity(const CrString& name);

	void SetParent(CrEntity* const parent);

	void SetLocalTransform(const CrTransform& localTransform);

private:

	CrString m_name;

	CrEntity* m_parent;
	//CrTransform* m_worldTransform;
	CrTransform* m_localTransform;
};