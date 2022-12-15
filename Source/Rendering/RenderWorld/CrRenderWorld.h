#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/Containers/CrVectorSet.h"
#include "Core/Containers/CrFixedVector.h"
#include "Core/Containers/CrArray.h"
#include "Core/Containers/CrHashSet.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrRenderModelInstance.h"

#include "Math/CrHlslppMatrixFloat.h"

class CrRenderModelInstance;
using CrRenderModelInstanceHandle = CrSharedPtr<CrRenderModelInstance>;

class CrModelInstanceIndexDummy;
using CrModelInstanceIndex = CrTypedId<CrModelInstanceIndexDummy, uint32_t>;

class CrCPUStackAllocator;

struct CrStandardSortKey
{
	CrStandardSortKey() {}

	CrStandardSortKey(uint32_t depthUint, const ICrGraphicsPipeline* pipeline, const CrRenderMesh* renderMesh, const CrMaterial* material)
	{
		// Set up sort key
		// Sorting is implementing in ascending order, so a lower depth sorts first

		depthKey    = (uint16_t)(depthUint >> 15); // Take top bits but don't include sign
		pipelineKey = (uint16_t)((uintptr_t)pipeline >> 3); // Remove last 3 bits which are likely to be equal
		meshKey     = (uint16_t)((uintptr_t)renderMesh >> 3);
		materialKey = (uint16_t)((uintptr_t)material >> 3);
	}

	union
	{
		struct
		{
			uint64_t depthKey    : 16;
			uint64_t meshKey     : 16;
			uint64_t materialKey : 16; // TODO Change to resource table
			uint64_t pipelineKey : 16;
		};

		uint64_t key = 0;
	};
};

// Everything that is needed to render a packet
struct CrRenderPacket
{
	bool operator < (const CrRenderPacket& other) const { return sortKey.key < other.sortKey.key; }

	CrStandardSortKey sortKey;

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
		Forward,
		GBuffer,

		// Editor Render Lists
		MouseSelection,
		EdgeSelection,
		Count
	};
};

// A collection of render packets to be rendered
struct CrRenderList
{
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

	CrVector<CrRenderPacket> m_renderPackets;
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
class CrRenderWorld
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
	void SetTransform(CrModelInstanceIndex instanceId, const float4x4& transform) { m_modelInstanceTransforms[instanceId.id] = transform; }
	void SetTransform(CrModelInstanceId instanceId, const float4x4& transform) { SetTransform(GetModelInstanceIndex(instanceId), transform); }
	float4x4 GetTransform(CrModelInstanceIndex instanceId) const { return m_modelInstanceTransforms[instanceId.id]; }
	float4x4 GetTransform(CrModelInstanceId instanceId) const { return GetTransform(GetModelInstanceIndex(instanceId)); }

	void SetRenderModel(CrModelInstanceIndex instanceIndex, const CrRenderModelSharedHandle& renderModel) { m_renderModels[instanceIndex.id] = renderModel; }
	void SetRenderModel(CrModelInstanceId instanceId, const CrRenderModelSharedHandle& renderModel) { SetRenderModel(GetModelInstanceIndex(instanceId), renderModel); }
	const CrRenderModelSharedHandle& GetRenderModel(CrModelInstanceIndex instanceIndex) const { return m_renderModels[instanceIndex.id]; }
	const CrRenderModelSharedHandle& GetRenderModel(CrModelInstanceId instanceId) const { return GetRenderModel(GetModelInstanceIndex(instanceId)); }

	// Editor properties
	void SetSelected(CrModelInstanceId instanceId);

	void AddSelected(CrModelInstanceId instanceId);

	void RemoveSelected(CrModelInstanceId instanceId);

	void ToggleSelected(CrModelInstanceId instanceId);

	bool GetSelected(CrModelInstanceId instanceId) const;

	void ClearSelection();

	void SetCamera(const CrCameraHandle& camera) { m_camera = camera; }

	const CrRenderList& GetRenderList(CrRenderListUsage::T usage) const { return m_renderLists[usage]; }

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

	void BeginRendering(const CrSharedPtr<CrCPUStackAllocator>& renderingStream);

	void EndRendering();

	// Editor

	void SetMouseSelectionEnabled(bool enable, const CrRectangle& boundingRectangle);

	bool GetMouseSelectionEnabled() const;

private:

	CrModelInstanceIndex GetModelInstanceIndex(CrModelInstanceId instanceId) const
	{ return m_modelInstanceIdToIndex[instanceId.id]; }

	CrModelInstanceId GetModelInstanceId(CrModelInstanceIndex instanceIndex) const
	{ return m_modelInstanceIndexToId[instanceIndex.id]; }

	// Model Instance Data

	CrVector<float4x4>                  m_modelInstanceTransforms;

	CrVector<CrRenderModelSharedHandle> m_renderModels;

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

	CrSharedPtr<CrCPUStackAllocator> m_renderingStream;

	// Camera data. We aren't doing data driven design for cameras as there won't be many
	// and it's easier to manage this way
	//CrVector<CrCamera> m_cameras;

	// TODO Fix single camera
	CrCameraHandle m_camera;

	// Render lists containing visible rendering packets

	CrRenderList m_renderLists[CrRenderListUsage::Count];

	// Visible model instances
	CrVector<CrModelInstanceIndex> m_visibleModelInstances;

	// Editor support

	CrHashSet<CrModelInstanceId::type> m_selectedInstances;

	bool m_computeMouseSelection = false;

	CrRectangle m_mouseSelectionBoundingRectangle;
};