#pragma once

#include "Core/CrTypedId.h"

#include "Rendering/CrVisibility.h"

#include "Math/CrHlslppMatrixFloatType.h"

#include "crstl/intrusive_ptr.h"

class CrModelInstance;
using CrModelInstanceId = CrTypedId<CrModelInstance, uint32_t>;

// Properties that only the editor is allowed to modify and won't be available in-game
struct CrEditorProperties
{
	// Instance belongs to the editor (doesn't do edge highlight, is not selectable, etc)
	bool isEditorBuiltin = false;

	// Mesh has a constant size on screen
	bool isConstantSizeOnScreen = false;

	// Mesh needs to go through the editor edge highlight
	bool isEdgeHighlight = false;
};

class CrModelInstance
{
public:

	CrModelInstance() 
		: m_transform(float4x4::identity())
		, m_visibilityId(0xffffffff)
	{}

	void SetTransform(const float4x4& transform) { m_transform = transform; }
	const float4x4& GetTransform() const { return m_transform; }

	void SetPosition(float3 position) { m_transform[3].xyz = position; }
	float3 GetPosition() const { return m_transform[3].xyz; }

	void SetRenderModel(const CrRenderModelHandle& renderModel) { m_renderModel = renderModel; }
	const CrRenderModelHandle& GetRenderModel() const { return m_renderModel; }

	void SetVisibilityId(uint32_t visibilityId) { m_visibilityId = visibilityId; }

#if defined(CR_EDITOR)
	
	// By default all entities are selectable in the editor, so we need to exclude manipulators, icons and other editor entities
	void SetIsEditorBuiltin(bool isEditorBuiltin) { m_editorProperties.isEditorBuiltin = isEditorBuiltin; }
	bool GetIsEditorBuiltin() const { return m_editorProperties.isEditorBuiltin; }

	void SetIsConstantSizeOnScreen(bool isConstantSizeOnScreen) { m_editorProperties.isConstantSizeOnScreen = isConstantSizeOnScreen; }
	bool GetIsConstantSizeOnScreen() const { return m_editorProperties.isConstantSizeOnScreen; }

	void SetIsEdgeHighlight(bool isEdgeHighlight) { m_editorProperties.isEdgeHighlight = isEdgeHighlight; }
	bool GetIsEdgeHighlight() const { return m_editorProperties.isEdgeHighlight; }

#endif

private:

	float4x4 m_transform;

	CrRenderModelHandle m_renderModel;

	CrBoundingBox m_boundingBox;

	uint32_t m_visibilityId;

#if defined(CR_EDITOR)
	CrEditorProperties m_editorProperties;
#endif
};
