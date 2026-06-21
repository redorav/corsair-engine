#pragma once

#include "Math/CrHlslppVectorFloatType.h"
#include "Math/CrHlslppQuaternionType.h"

#include "crstl/string.h"
#include "crstl/vector.h"

class CrTransform;

namespace EntityType
{
	enum T : uint32_t
	{
		ModelInstance,
		Light,
		Camera,
		Count
	};
}

struct CrEntityID
{
	CrEntityID() : type(EntityType::Count), instanceID(0) {}

	explicit CrEntityID(uint32_t key) : key(key) {}

	union
	{
		struct
		{
			EntityType::T type : 8;
			uint32_t instanceID : 24;
		};

		uint32_t key;
	};
};

static_assert(sizeof(CrEntityID) == sizeof(uint32_t), "");

class CrEntity
{
public:

	// TODO Put in transform
	float3 m_position;
	float3 m_scale;
	quaternion m_qrotation;

	crstl::vector<CrEntity*> m_entities;

	CrEntity();

	CrEntity(const crstl::string& name);

	const float3& GetPosition() const { return m_position; }

	const float3& GetScale() const { return m_scale; }

	const quaternion& GetRotation() const { return m_qrotation; }

	void SetParent(CrEntity* const parent);

	void SetEntityID(CrEntityID entityID) { m_entityID = entityID; }

	CrEntityID GetEntityID() const { return m_entityID; }

protected:

	crstl::string m_name;

	// Contains the type and unique ID
	CrEntityID m_entityID;

	CrEntity* m_parent;
};