#include "CrRendering_pch.h"

#include "Rendering/CrRenderWorld.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrRenderMesh.h"
#include "Rendering/CrCamera.h"

#include "Rendering/CrCPUStackAllocator.h"

#include "Core/CrSort.h"

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/Logging/ICrDebug.h"

CrRenderWorld::CrRenderWorld()
{
	// TODO Make sure when we create a model instance all of these are updated
	m_modelInstanceTransforms.resize(1000);
	m_renderModels.resize(1000);
	m_modelInstanceObbs.resize(1000);

	// Defaults to invalid id
	m_modelInstanceIdToIndex.resize(1000);
	m_modelInstanceIndexToId.resize(1000);

	m_modelInstanceEditorProperties.resize(1000);

	m_maxModelInstanceId = CrModelInstanceId(0);
	m_numModelInstances = CrModelInstanceIndex(0);
}

CrRenderWorld::~CrRenderWorld()
{
	//CrAssertMsg(m_numModelInstances.id == 0, "Not all model instances were destroyed correctly");
}

CrRenderModelInstance CrRenderWorld::CreateModelInstance()
{
	CrModelInstanceId availableId;

	// If we have an available id (from a previously deleted instance) reuse that
	if (m_lastAvailableId != CrModelInstanceId())
	{
		availableId = m_lastAvailableId;
		m_lastAvailableId.id = m_modelInstanceIdToIndex[m_lastAvailableId.id].id;
	}
	else // Otherwise create a new id
	{
		availableId = m_maxModelInstanceId;
		m_maxModelInstanceId.id++;
	}

	// Initialize some members to sensible defaults
	m_modelInstanceTransforms[m_numModelInstances.id] = float4x4::identity();

	// Initialize remapping tables
	m_modelInstanceIdToIndex[availableId.id] = CrModelInstanceIndex(m_numModelInstances.id);
	m_modelInstanceIndexToId[m_numModelInstances.id] = CrModelInstanceId(availableId.id);

	m_numModelInstances.id++;

	return CrRenderModelInstance(availableId);
}

void CrRenderWorld::DestroyModelInstance(CrModelInstanceId instanceId)
{
	CrAssertMsg(instanceId.id < CrModelInstanceId::MaxId, "Invalid model instance id");
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
	m_modelInstanceTransforms[destroyedInstanceIndex.id]       = m_modelInstanceTransforms[lastInstanceIndex.id];
	m_renderModels[destroyedInstanceIndex.id]                  = m_renderModels[lastInstanceIndex.id];
	m_modelInstanceObbs[destroyedInstanceIndex.id]             = m_modelInstanceObbs[lastInstanceIndex.id];
	m_modelInstanceEditorProperties[destroyedInstanceIndex.id] = m_modelInstanceEditorProperties[lastInstanceIndex.id];

	// Free references (no need to zero out data that doesn't have a smart pointer)
	m_renderModels[lastInstanceIndex.id]   = nullptr;

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
	m_modelInstanceIndexToId[lastInstanceIndex.id]      = CrModelInstanceId();
	
	// Decrement number of model instances
	m_numModelInstances.id--;
}

void CrRenderWorld::SetSelected(CrModelInstanceIndex instanceIndex, bool isSelected)
{
	m_modelInstanceEditorProperties[instanceIndex.id].isSelected = isSelected;
}

void CrRenderWorld::SetSelected(CrModelInstanceId instanceId, bool isSelected)
{
	SetSelected(GetModelInstanceIndex(instanceId), isSelected);
}

bool CrRenderWorld::GetSelected(CrModelInstanceIndex instanceIndex) const
{
	return m_modelInstanceEditorProperties[instanceIndex.id].isSelected;
}

bool CrRenderWorld::GetSelected(CrModelInstanceId instanceId) const
{
	return GetSelected(GetModelInstanceIndex(instanceId));
}

void CrRenderWorld::ComputeVisibilityAndRenderPackets()
{
	m_visibleModelInstances.clear();

	for (CrModelInstanceIndex instanceIndex(0); instanceIndex < m_numModelInstances; ++instanceIndex)
	{
		const CrRenderModelSharedHandle& renderModel = GetRenderModel(instanceIndex);
		const CrBoundingBox& modelBoundingBox = renderModel->GetBoundingBox();
		float4x4 transform = GetTransform(instanceIndex);

		uint32_t meshCount = renderModel->GetRenderMeshCount();

		// Check for instance visibility. Only check if number of instances > 1, otherwise we duplicate work
		if (meshCount > 1)
		{
			CrBoxVertices modelProjectedCorners;
			CrVisibility::ComputeObbProjection(modelBoundingBox, transform, m_camera->GetWorld2ProjectionMatrix(), modelProjectedCorners);

			if (!CrVisibility::AreProjectedPointsOnScreen(modelProjectedCorners))
			{
				continue;
			}
		}

		// Allocate more transforms depending on what the model instance provides
		float4x4* transforms = m_renderingStream->Allocate<float4x4>(1).memory;
		transforms[0] = transform;

		m_visibleModelInstances.push_back(instanceIndex);

		bool isEditorSelected = GetSelected(instanceIndex);

		for (uint32_t meshIndex = 0; meshIndex < meshCount; ++meshIndex)
		{
			const auto& meshMaterial       = renderModel->GetRenderMeshMaterial(meshIndex);
			const CrRenderMesh* renderMesh = meshMaterial.first.get();
			const CrMaterial* material     = meshMaterial.second.get();

			const CrBoundingBox& meshBoundingBox = renderMesh->GetBoundingBox();

			CrBoxVertices meshProjectedCorners;
			CrVisibility::ComputeObbProjection(meshBoundingBox, transform, m_camera->GetWorld2ProjectionMatrix(), meshProjectedCorners);

			// Compute mesh visibility and don't render if outside frustum
			if (!CrVisibility::AreProjectedPointsOnScreen(meshProjectedCorners))
			{
				continue;
			}

			float3 obbCenterWorld = mul(float4(meshBoundingBox.center, 1.0f), transform).xyz;

			float3 cameraToMesh = obbCenterWorld - m_camera->GetPosition();

			float squaredDistance = dot(cameraToMesh, cameraToMesh);

			uint32_t depthUint = *reinterpret_cast<uint32_t*>(&squaredDistance);

			// TODO How to do fading?
			// TODO How to do LOD selection?

			CrRenderPacket mainPacket;
			mainPacket.transforms   = transforms;
			mainPacket.renderMesh   = renderMesh;
			mainPacket.material     = material;
			mainPacket.numInstances = 1;

			// Create render packets and add to the render lists
			mainPacket.pipeline = renderModel->GetPipeline(meshIndex, CrMaterialPipelineVariant::Transparency).get();
			mainPacket.sortKey  = CrStandardSortKey(depthUint, mainPacket.pipeline, renderMesh, material);
			m_renderLists[CrRenderListUsage::Forward].AddPacket(mainPacket);

			mainPacket.pipeline = renderModel->GetPipeline(meshIndex, CrMaterialPipelineVariant::GBuffer).get();
			mainPacket.sortKey  = CrStandardSortKey(depthUint, mainPacket.pipeline, renderMesh, material);
			m_renderLists[CrRenderListUsage::GBuffer].AddPacket(mainPacket);

			if (isEditorSelected)
			{
				mainPacket.pipeline = renderModel->GetPipeline(meshIndex, CrMaterialPipelineVariant::Debug).get();
				mainPacket.sortKey = CrStandardSortKey(depthUint, mainPacket.pipeline, renderMesh, material);
				m_renderLists[CrRenderListUsage::EdgeSelection].AddPacket(mainPacket);
			}
		}
	}

	// Sort the render lists
	for (CrRenderList& renderList : m_renderLists)
	{
		renderList.Sort();
	}
}

void CrRenderWorld::BeginRendering(const CrSharedPtr<CrCPUStackAllocator>& renderingStream)
{
	m_renderingStream = renderingStream;
}

void CrRenderWorld::EndRendering()
{
	for (CrRenderList& renderList : m_renderLists)
	{
		renderList.Clear();
	}
}

void CrRenderList::Clear()
{
	m_renderPackets.clear();
}

void CrRenderList::Sort()
{
	CrQuicksort(m_renderPackets.begin(), m_renderPackets.end());
}
