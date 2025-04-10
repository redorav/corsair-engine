#pragma once

#include "Math/CrHlslppVectorFloatType.h"
#include "Math/CrHlslppQuaternionType.h"

#include "crstl/string.h"
#include "crstl/vector.h"

class CrTransform;

class CrEntity
{
public:

	bool isActive = true;

	// TODO Put in transform
	float3 m_position;
	float3 m_scale;
	quaternion m_qrotation;

	crstl::vector<CrEntity*> gameObjects;

	CrEntity();
	CrEntity(const crstl::string& name);

	const float3& GetPosition() const { return m_position; }

	const float3& GetScale() const { return m_scale; }

	const quaternion& GetRotation() const { return m_qrotation; }

	void SetParent(CrEntity* const parent);

	void SetLocalTransform(const CrTransform& localTransform);

private:

	crstl::string m_name;

	CrEntity* m_parent;
	//CrTransform* m_worldTransform;
	CrTransform* m_localTransform;
};