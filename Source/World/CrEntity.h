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

// Properties that only the editor is allowed to modify and won't be available in-game
struct EditorProperties
{
	// Instance belongs to the editor (doesn't do edge highlight, is not selectable, etc)
	bool isEditorBuiltin = false;

	// Mesh has a constant size on screen
	bool isConstantSizeOnScreen = false;

	// Mesh needs to go through the editor edge highlight
	bool isEdgeHighlight = false;
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

#if defined(CR_EDITOR)

	// By default all entities are selectable in the editor, so we need to exclude manipulators, icons and other editor entities
	void SetIsEditorBuiltin(bool isEditorBuiltin) { m_editorProperties.isEditorBuiltin = isEditorBuiltin; }
	bool GetIsEditorBuiltin() const { return m_editorProperties.isEditorBuiltin; }

	void SetIsConstantSizeOnScreen(bool isConstantSizeOnScreen) { m_editorProperties.isConstantSizeOnScreen = isConstantSizeOnScreen; }
	bool GetIsConstantSizeOnScreen() const { return m_editorProperties.isConstantSizeOnScreen; }

	void SetIsEdgeHighlight(bool isEdgeHighlight) { m_editorProperties.isEdgeHighlight = isEdgeHighlight; }
	bool GetIsEdgeHighlight() const { return m_editorProperties.isEdgeHighlight; }

#endif

protected:

	crstl::string m_name;

	// Contains the type and unique ID
	crntt::EntityID m_entityID;

	CrEntity* m_parent;

#if defined(CR_EDITOR)

	EditorProperties m_editorProperties;

#endif
};