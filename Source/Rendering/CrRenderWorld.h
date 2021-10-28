#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/Containers/CrVectorSet.h"
#include "Core/Containers/CrFixedVector.h"
#include "Core/Containers/CrArray.h"
#include "Core/Containers/CrBitSet.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrRenderModelInstance.h"

class CrRenderModelInstance;
using CrRenderModelInstanceHandle = CrSharedPtr<CrRenderModelInstance>;

class CrModelInstanceIndexDummy;
using CrModelInstanceIndex = CrTypedId<CrModelInstanceIndexDummy, uint32_t>;

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

	static const uint32_t InvalidModelInstanceId = 0xffffffff;

	CrRenderWorld();

	~CrRenderWorld();

	// Allocate model instance in the world
	CrRenderModelInstance CreateModelInstance();

	// Frees model instance from the world, with all its resources
	// Only the ModelInstance class can call this (when it goes out of scope)
	void DestroyModelInstance(CrModelInstanceId instanceId);

	// Set properties on the model instance, either through the instance id or the instance index
	void SetTransform(CrModelInstanceIndex instanceId, float4x4 transform) { m_modelInstanceTransforms[instanceId.id] = transform; }
	void SetTransform(CrModelInstanceId instanceId, float4x4 transform) { SetTransform(GetModelInstanceIndex(instanceId), transform); }
	float4x4 GetTransform(CrModelInstanceIndex instanceId) const { return m_modelInstanceTransforms[instanceId.id]; }
	float4x4 GetTransform(CrModelInstanceId instanceId) const { return GetTransform(GetModelInstanceIndex(instanceId)); }

	void SetRenderModel(CrModelInstanceIndex instanceIndex, const CrRenderModelSharedHandle& renderModel) { m_renderModels[instanceIndex.id] = renderModel; }
	void SetRenderModel(CrModelInstanceId instanceId, const CrRenderModelSharedHandle& renderModel) { SetRenderModel(GetModelInstanceIndex(instanceId), renderModel); }
	const CrRenderModelSharedHandle& GetRenderModel(CrModelInstanceIndex instanceIndex) const { return m_renderModels[instanceIndex.id]; }
	const CrRenderModelSharedHandle& GetRenderModel(CrModelInstanceId instanceId) const { return GetRenderModel(GetModelInstanceIndex(instanceId)); }

	// Traverse the model instances
	template<typename Fn>
	void ForEachModelInstance(const Fn& fn) const
	{
		for (CrModelInstanceIndex instanceIndex(0); instanceIndex < m_numModelInstances; ++instanceIndex)
		{
			fn(this, instanceIndex);
		}
	}

private:

	CrModelInstanceIndex GetModelInstanceIndex(CrModelInstanceId instanceId) const
	{ return m_modelInstanceIdToIndex[instanceId.id]; }

	CrModelInstanceId GetModelInstanceId(CrModelInstanceIndex instanceIndex) const
	{ return m_modelInstanceIndexToId[instanceIndex.id]; }

	// Model Instance Data

	CrVector<float4x4> m_modelInstanceTransforms;

	CrVector<CrRenderModelSharedHandle> m_renderModels;

	CrVector<CrGraphicsPipelineHandle> m_pipelines;

	CrVector<CrBoundingBox> m_modelInstanceObbs;

	CrVector<CrModelInstanceIndex> m_modelInstanceIdToIndex;

	CrVector<CrModelInstanceId> m_modelInstanceIndexToId;

	CrModelInstanceId m_maxModelInstanceId;

	CrModelInstanceIndex m_numModelInstances;

	CrModelInstanceId m_lastAvailableId;

	// Lights Data
	CrVector<float4x4> m_lightTransforms;

	CrVector<CrBoundingBox> m_lightObbs;

	// TODO light ids

	// Camera data. We aren't doing data driven design for cameras as there won't be many
	// and it's easier to manage this way
	//CrVector<CrCamera> m_cameras;
};