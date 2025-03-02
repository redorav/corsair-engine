#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/Containers/CrFixedVector.h"
#include "Core/Containers/CrHashMap.h"

#include "Core/SmartPointers/CrIntrusivePtr.h"
#include "Core/Logging/ICrDebug.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/RenderWorld/CrRenderModelInstance.h"

#include "Math/CrHlslppMatrixFloatType.h"

class CrModelInstanceIndexDummy;
using CrModelInstanceIndex = CrTypedId<CrModelInstanceIndexDummy, uint32_t>;

class CrCPUStackAllocator;

typedef uint64_t CrSortKey;

// Everything that is needed to render a packet
struct CrRenderPacket
{
	bool operator < (const CrRenderPacket& other) const { return sortKey < other.sortKey; }

	CrSortKey sortKey;

	float4x4* transforms;
	const CrRenderMesh* renderMesh;
	const CrMaterial* material; // TODO Replace with resource table (textures, constants, etc)
	const ICrGraphicsPipeline* pipeline;
	const void* extra = nullptr; // Use this to piggyback extra data

	// This decides how many instances are rendered of this mesh, and also
	// how many transforms are in the transform array
	uint32_t numInstances;
};

// Render list names. Each render list is populated when processing the model instance
namespace CrRenderListUsage
{
	enum T
	{
		GBuffer,
		Transparency,

		// Editor Render Lists
		MouseSelection,
		EdgeSelection,
		Count
	};
};

// A collection of render packets to be rendered
struct CrRenderList
{
	CrRenderList() {}

	void AddPacket(const CrRenderPacket& renderPacket)
	{
		m_renderPackets.push_back(renderPacket);
	}

	size_t Size() const { return m_renderPackets.size(); }

	void Clear();

	void Sort();

	template<typename FunctionT>
	void ForEachRenderPacket(const FunctionT& function) const
	{
		for (const CrRenderPacket& renderPacket : m_renderPackets)
		{
			function(renderPacket);
		}
	}

private:

	CrRenderList(const CrRenderList& other) = delete;

	CrVector<CrRenderPacket> m_renderPackets;
};

// Properties that only the editor is allowed to modify and won't be available in-game
struct CrEditorProperties
{
	// Mesh has a constant size on screen
	bool isConstantSizeOnScreen = false;

	// Mesh needs to go through the editor edge highlight
	bool isEdgeHighlight = false;
};

// CrRenderWorld is where all rendering primitives live, e.g. model instances,
// cameras, lights and other entities that contribute to the way the frame is rendered
// such as post effects, etc. The render world is able to create and manage the members
// of these entities until the time they are destroyed.
//
// The idea behind the render world is to make it aware of creation and destruction of
// its entities. In the case of entities that we expect to have hundreds or thousands of,
// this allows us to create efficient layouts and access patterns that make creation,
// deletion and traversal fast and convenient.
//
// The render world also computes visibility and creates rendering packets that encapsulate
// everything that is needed to render an object. After visibility, there is a list of
// visible packets with their state all ready to be translated into drawcalls.
class CrRenderWorld final : public CrIntrusivePtrInterface
{
public:

	CrRenderWorld();

	~CrRenderWorld();

	// Allocate model instance in the world
	CrRenderModelInstance CreateModelInstance();

	// Frees model instance from the world, with all its resources
	// Only the ModelInstance class can call this (when it goes out of scope)
	void DestroyModelInstance(CrModelInstanceId instanceId);

	// Set properties on the model instance, either through the instance id or the instance index
	void SetTransform(CrModelInstanceIndex instanceIndex, const float4x4& transform) { m_modelInstanceTransforms[instanceIndex.id] = transform; }
	void SetTransform(CrModelInstanceId instanceId, const float4x4& transform) { SetTransform(GetModelInstanceIndex(instanceId), transform); }
	float4x4 GetTransform(CrModelInstanceIndex instanceIndex) const { return m_modelInstanceTransforms[instanceIndex.id]; }
	float4x4 GetTransform(CrModelInstanceId instanceId) const { return GetTransform(GetModelInstanceIndex(instanceId)); }

	void SetPosition(CrModelInstanceId instanceId, float3 position) { m_modelInstanceTransforms[GetModelInstanceIndex(instanceId).id][3].xyz = position; }
	float3 GetPosition(CrModelInstanceId instanceId) { return m_modelInstanceTransforms[GetModelInstanceIndex(instanceId).id][3].xyz; }

	void SetRenderModel(CrModelInstanceIndex instanceIndex, const CrRenderModelHandle& renderModel)
	{
		CrAssertMsg(renderModel != nullptr, "Render model cannot be null");
		m_renderModels[instanceIndex.id] = renderModel;
	}

	void SetRenderModel(CrModelInstanceId instanceId, const CrRenderModelHandle& renderModel) { SetRenderModel(GetModelInstanceIndex(instanceId), renderModel); }
	const CrRenderModelHandle& GetRenderModel(CrModelInstanceIndex instanceIndex) const { return m_renderModels[instanceIndex.id]; }
	const CrRenderModelHandle& GetRenderModel(CrModelInstanceId instanceId) const { return GetRenderModel(GetModelInstanceIndex(instanceId)); }

	void SetCamera(const CrCameraHandle& camera);
	const CrCameraHandle& GetCamera() const { return m_camera; }

	const CrRenderList& GetRenderList(CrRenderListUsage::T usage) const { return m_renderLists[usage]; }

	bool HasRenderList(CrRenderListUsage::T usage) const { return m_renderLists[usage].Size() > 0; }

	// Traverse the model instances
	template<typename FunctionT>
	void ForEachModelInstance(const FunctionT& function) const
	{
		for (CrModelInstanceIndex instanceIndex(0); instanceIndex < m_numModelInstances; ++instanceIndex)
		{
			function(this, instanceIndex);
		}
	}

	void ComputeVisibilityAndRenderPackets();

	// Traverse visible model instances
	template<typename FunctionT>
	void ForEachVisibleModelInstance(const FunctionT& function) const
	{
		for (uint32_t visibleIndex = 0; visibleIndex < m_visibleModelInstances.size(); ++visibleIndex)
		{
			CrModelInstanceIndex instanceIndex = m_visibleModelInstances[visibleIndex];
			function(this, instanceIndex, visibleIndex);
		}
	}

	void BeginRendering(const CrIntrusivePtr<CrCPUStackAllocator>& renderingStream);

	void EndRendering();

private:

	CrModelInstanceIndex GetModelInstanceIndex(CrModelInstanceId instanceId) const
	{ return m_modelInstanceIdToIndex[instanceId.id]; }

	CrModelInstanceId GetModelInstanceId(CrModelInstanceIndex instanceIndex) const
	{ return m_modelInstanceIndexToId[instanceIndex.id]; }

	// Model Instance Data

	CrVector<float4x4>                  m_modelInstanceTransforms;

	CrVector<CrRenderModelHandle>       m_renderModels;

	CrVector<CrBoundingBox>             m_modelInstanceObbs;

	CrVector<CrModelInstanceIndex>      m_modelInstanceIdToIndex;

	CrVector<CrModelInstanceId>         m_modelInstanceIndexToId;

	CrModelInstanceId                   m_maxModelInstanceId;

	CrModelInstanceIndex                m_numModelInstances;

	CrModelInstanceId                   m_lastAvailableId;

	// Lights Data
	CrVector<float4x4> m_lightTransforms;

	CrVector<CrBoundingBox> m_lightObbs;

	// TODO light ids

	CrIntrusivePtr<CrCPUStackAllocator> m_renderingStream;

	// Camera data. We aren't doing data driven design for cameras as there won't be many
	// and it's easier to manage this way
	//CrVector<CrCamera> m_cameras;

	// TODO Fix single camera
	CrCameraHandle m_camera;

	// Render lists containing visible rendering packets

	CrRenderList m_renderLists[CrRenderListUsage::Count];

	// Visible model instances
	CrVector<CrModelInstanceIndex> m_visibleModelInstances;

#if defined(CR_EDITOR)

public:

	void SetIsEditorEdgeHighlight(CrModelInstanceId instanceId, bool value);

	bool GetIsEditorEdgeHighlight(CrModelInstanceId instanceId) const;

	// By default all entities are selectable in the editor, so we need to exclude
	// manipulators, icons and other editor entities
	void SetEditorInstance(CrModelInstanceId instanceId);

	bool GetIsEditorInstance(CrModelInstanceId instanceId) const;

	void SetConstantSize(CrModelInstanceIndex instanceIndex, bool constantSize) { m_editorProperties[instanceIndex.id].isConstantSizeOnScreen = constantSize; }

	void SetConstantSize(CrModelInstanceId instanceId, bool constantSize) { SetConstantSize(GetModelInstanceIndex(instanceId), constantSize); }

	bool GetConstantSize(CrModelInstanceIndex instanceIndex) const { return m_editorProperties[instanceIndex.id].isConstantSizeOnScreen; }

	bool GetConstantSize(CrModelInstanceId instanceId) const { return GetConstantSize(GetModelInstanceIndex(instanceId)); }

	void SetMouseSelectionEnabled(bool enable, const CrRectangle& boundingRectangle);

	bool GetMouseSelectionEnabled() const;

private:

	// Make sure we can exclude editor entities from all the standard behavior
	// such as selection highlight
	CrHashSet<CrModelInstanceId::type> m_editorInstances;

	CrVector<CrEditorProperties>       m_editorProperties;

	bool m_computeMouseSelection = false;

	CrRectangle m_mouseSelectionBoundingRectangle;

#endif
};