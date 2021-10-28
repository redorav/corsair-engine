#include "CrRendering_pch.h"

#include "Rendering/CrRenderWorld.h"
#include "Rendering/CrRenderModel.h"

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/Logging/ICrDebug.h"

CrRenderWorld::CrRenderWorld()
{
	m_modelInstanceTransforms.resize(1000);
	m_renderModels.resize(1000);
	m_pipelines.resize(1000);
	m_modelInstanceObbs.resize(1000);

	m_modelInstanceIdToIndex.resize(1000);
	m_modelInstanceIndexToId.resize(1000);

	for (uint32_t i = 0; i < m_modelInstanceIdToIndex.size(); ++i)
	{
		m_modelInstanceIdToIndex[i] = CrModelInstanceIndex(InvalidModelInstanceId);
		m_modelInstanceIndexToId[i] = CrModelInstanceId(InvalidModelInstanceId);
	}

	m_lastAvailableId = CrModelInstanceId(InvalidModelInstanceId);
}

CrRenderWorld::~CrRenderWorld()
{
	CrAssertMsg(m_numModelInstances.id == 0, "Not all model instances were destroyed correctly");
}

CrRenderModelInstance CrRenderWorld::CreateModelInstance()
{
	CrModelInstanceId availableId = CrModelInstanceId(InvalidModelInstanceId);

	// If we have an available id (from a previously deleted instance) reuse that
	if (m_lastAvailableId.id != InvalidModelInstanceId)
	{
		availableId = m_lastAvailableId;
		m_lastAvailableId.id = m_modelInstanceIdToIndex[m_lastAvailableId.id].id;
	}
	else // Otherwise create a new id
	{
		availableId = m_maxModelInstanceId;
		m_maxModelInstanceId.id++;
	}

	// Initialize some members top sensible defaults
	m_modelInstanceTransforms[m_numModelInstances.id] = float4x4::identity();

	// Initialize remapping tables
	m_modelInstanceIdToIndex[availableId.id] = CrModelInstanceIndex(m_numModelInstances.id);
	m_modelInstanceIndexToId[m_numModelInstances.id] = CrModelInstanceId(availableId.id);

	m_numModelInstances.id++;

	return CrRenderModelInstance(availableId);
}

void CrRenderWorld::DestroyModelInstance(CrModelInstanceId instanceId)
{
	CrAssertMsg(instanceId.id >= 0, "Invalid model instance id");
	CrAssertMsg(m_numModelInstances.id > 0, "Destroying more model instances than were created");

	// Instance id and index of the model instance about to be destroyed
	CrModelInstanceId destroyedInstanceId       = instanceId;
	CrModelInstanceIndex destroyedInstanceIndex = GetModelInstanceIndex(destroyedInstanceId);

	// Instance id and index of the model instance located at the end of the list (which we're about to swap)
	CrModelInstanceIndex lastInstanceIndex      = m_numModelInstances - 1;
	CrModelInstanceId lastInstanceId            = GetModelInstanceId(lastInstanceIndex);
	
	//---------------
	// Swap resources
	//---------------

	// We want them always well packed. Take the index where the data lived for the destroyed model instance
	// and copy the data belonging to the last model instance in the array
	m_modelInstanceTransforms[destroyedInstanceIndex.id] = m_modelInstanceTransforms[lastInstanceIndex.id];
	m_renderModels[destroyedInstanceIndex.id]            = m_renderModels[lastInstanceIndex.id];
	m_pipelines[destroyedInstanceIndex.id]               = m_pipelines[lastInstanceIndex.id];
	m_modelInstanceObbs[destroyedInstanceIndex.id]       = m_modelInstanceObbs[lastInstanceIndex.id];

	// Free references (no need to zero out data that doesn't have a smart pointer)
	m_renderModels[lastInstanceIndex.id]   = nullptr;
	m_pipelines[lastInstanceIndex.id]      = nullptr;

	//--------------------------
	// Update indirection tables
	//--------------------------

	// Update the last instance id to point to the index of the destroyed instance
	m_modelInstanceIdToIndex[lastInstanceId.id]      = CrModelInstanceIndex(destroyedInstanceIndex.id);

	// Store the last available id in the destroyed instance id's slot
	// Cast the data here even though it's not correct (storing instance index in place of instance id)
	m_modelInstanceIdToIndex[destroyedInstanceId.id] = CrModelInstanceIndex(m_lastAvailableId.id);

	// Update last available id (in a linked list fashion)
	m_lastAvailableId = destroyedInstanceId;

	// Point destroyed instance index (where the last model instance now lives) to its model index
	m_modelInstanceIndexToId[destroyedInstanceIndex.id] = lastInstanceId;

	// Point last model instance to an invalid id
	m_modelInstanceIndexToId[lastInstanceIndex.id]      = CrModelInstanceId(0xffffffff);
	
	// Decrement number of model instances
	m_numModelInstances.id--;
}