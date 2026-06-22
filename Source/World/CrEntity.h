#pragma once

#include "Math/CrHlslppVectorFloatType.h"
#include "Math/CrHlslppQuaternionType.h"

#include "crstl/string.h"
#include "crstl/vector.h"

namespace crntt
{
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

	struct EntityID
	{
		EntityID() : type(EntityType::Count), instanceID(0) {}

		explicit EntityID(uint32_t key) : key(key) {}

		union
		{
			struct
			{
				uint32_t instanceID : 24;
				EntityType::T type : 8;
			};

			uint32_t key;
		};
	};

	static_assert(sizeof(EntityID) == sizeof(uint32_t), "");
};

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

	void SetEntityID(crntt::EntityID entityID) { m_entityID = entityID; }

	crntt::EntityID GetEntityID() const { return m_entityID; }

protected:

	crstl::string m_name;

	// Contains the type and unique ID
	crntt::EntityID m_entityID;

	CrEntity* m_parent;
};