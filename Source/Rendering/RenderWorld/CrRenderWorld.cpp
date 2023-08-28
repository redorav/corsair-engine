#include "Rendering/CrRendering_pch.h"

#include "Rendering/RenderWorld/CrRenderWorld.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrRenderMesh.h"
#include "Rendering/CrCamera.h"

#include "Rendering/CrCPUStackAllocator.h"

#include "Core/CrSort.h"

#include "Core/Logging/ICrDebug.h"

#define RENDER_WORLD_VALIDATION

#if defined(RENDER_WORLD_VALIDATION)
	#define CrRenderWorldAssertMsg(condition, message, ...) CrAssertMsg(condition, message, __VA_ARGS__)
#else
	#define CrRenderWorldAssertMsg(condition, message, ...)
#endif

CrRenderWorld::CrRenderWorld()
{
	// TODO Make sure when we create a model instance all of these are updated
	m_modelInstanceTransforms.resize(1000);
	m_renderModels.resize(1000);
	m_modelInstanceObbs.resize(1000);
	m_editorProperties.resize(1000);

	// Defaults to invalid id
	m_modelInstanceIdToIndex.resize(1000);
	m_modelInstanceIndexToId.resize(1000);

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
	m_editorProperties[destroyedInstanceIndex.id]              = m_editorProperties[lastInstanceIndex.id];

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

void CrRenderWorld::SetIsEditorEdgeHighlight(CrModelInstanceId instanceId, bool value)
{
	if (!GetIsEditorInstance(instanceId))
	{
		m_editorProperties[GetModelInstanceIndex(instanceId).id].isEdgeHighlight = value;
	}
}

bool CrRenderWorld::GetIsEditorEdgeHighlight(CrModelInstanceId instanceId) const
{
	return m_editorProperties[GetModelInstanceIndex(instanceId).id].isEdgeHighlight;
}

void CrRenderWorld::SetEditorInstance(CrModelInstanceId instanceId)
{
	m_editorInstances.insert(instanceId.id);
}

bool CrRenderWorld::GetIsEditorInstance(CrModelInstanceId instanceId) const
{
	return m_editorInstances.find(instanceId.id) != m_editorInstances.end();
}

void CrRenderWorld::SetCamera(const CrCameraHandle& camera)
{
	m_camera = camera;
}

void CrRenderWorld::ComputeVisibilityAndRenderPackets()
{
	m_visibleModelInstances.clear();

	for (CrModelInstanceIndex instanceIndex(0); instanceIndex < m_numModelInstances; ++instanceIndex)
	{
		const CrRenderModelHandle& renderModel = GetRenderModel(instanceIndex);
		const CrBoundingBox& modelBoundingBox = renderModel->GetBoundingBox();
		float4x4 transform = GetTransform(instanceIndex);

		CrRenderWorldAssertMsg(any(modelBoundingBox.extents != 0.0f), "Invalid bounding box extents");

		uint32_t meshCount = renderModel->GetRenderMeshCount();

		// If this mesh is set to do constant size (like for manipulators) we need to scale by the distance in Z to the camera
		if (GetConstantSize(instanceIndex))
		{
			float4 position = transform[3];
			float3 cameraToPosition = position.xyz - m_camera->GetPosition();
			float distanceToCamera = dot(cameraToPosition, m_camera->GetForwardVector());
			float4x4 scaleMtx = float4x4::scale(distanceToCamera);
			transform = mul(scaleMtx, transform);
			transform[3] = position;
		}

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

		CrModelInstanceId instanceId = GetModelInstanceId(instanceIndex);

		const CrEditorProperties& editorProperties = m_editorProperties[instanceIndex.id];

		bool isEditorEdgeHighlight = editorProperties.isEdgeHighlight;

		bool computeMouseSelection = GetMouseSelectionEnabled();

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

			const ICrGraphicsPipeline* transparencyPipeline = renderModel->GetPipeline(meshIndex, CrMaterialPipelineVariant::Transparency).get();
			const ICrGraphicsPipeline* gBufferPipeline      = renderModel->GetPipeline(meshIndex, CrMaterialPipelineVariant::GBuffer).get();
			const ICrGraphicsPipeline* debugPipeline        = renderModel->GetPipeline(meshIndex, CrMaterialPipelineVariant::Debug).get();

			if (gBufferPipeline)
			{
				mainPacket.pipeline = gBufferPipeline;
				mainPacket.sortKey = CrStandardSortKey(depthUint, mainPacket.pipeline, renderMesh, material);
				m_renderLists[CrRenderListUsage::GBuffer].AddPacket(mainPacket);
			}

			if (transparencyPipeline)
			{
				// Create render packets and add to the render lists
				mainPacket.pipeline = transparencyPipeline;
				mainPacket.sortKey = CrStandardSortKey(depthUint, mainPacket.pipeline, renderMesh, material);
				m_renderLists[CrRenderListUsage::Forward].AddPacket(mainPacket);
			}

#if defined(CR_EDITOR)

			if (computeMouseSelection)
			{
				// Compute bounding box in pixel space and check whether the mouse cursor is inside it
				// If it is, add to the mouse selection list. This can cause slowdowns during rendering
				// Speeding it up can also have the added benefit that we could continously do this process
				const float4 uvScale = float4(0.5f, -0.5f, 0.0f, 0.0f);
				const float4 uvBias = float4(0.5f, 0.5f, 0.0f, 0.0f);
				float4 uvPositionMin(+1.0e+100);
				float4 uvPositionMax(-1.0e+100);

				for (uint32_t i = 0; i < meshProjectedCorners.size(); ++i)
				{
					float4 screenPosition = meshProjectedCorners[i] / meshProjectedCorners[i].wwww;
					float4 uvPosition = screenPosition * uvScale + uvBias;
					uvPositionMin = min(uvPositionMin, uvPosition);
					uvPositionMax = max(uvPositionMax, uvPosition);
				}

				float4 resolution = float4(m_camera->GetResolutionWidth(), m_camera->GetResolutionHeight(), 1.0f, 1.0f);
				float4 pixelPositionMin = uvPositionMin * resolution;
				float4 pixelPositionMax = uvPositionMax * resolution;

				if (m_mouseSelectionBoundingRectangle.x >= pixelPositionMin.x &&
					m_mouseSelectionBoundingRectangle.x <= pixelPositionMax.x &&
					m_mouseSelectionBoundingRectangle.y >= pixelPositionMin.y &&
					m_mouseSelectionBoundingRectangle.y <= pixelPositionMax.y)
				{
					mainPacket.pipeline = debugPipeline;
					mainPacket.sortKey = CrStandardSortKey(depthUint, mainPacket.pipeline, renderMesh, material);
					mainPacket.extra = (void*)(uintptr_t)instanceId.id;
					m_renderLists[CrRenderListUsage::MouseSelection].AddPacket(mainPacket);
				}
			}

			if (isEditorEdgeHighlight)
			{
				mainPacket.pipeline = debugPipeline;
				mainPacket.sortKey = CrStandardSortKey(depthUint, mainPacket.pipeline, renderMesh, material);
				m_renderLists[CrRenderListUsage::EdgeSelection].AddPacket(mainPacket);
			}

#endif
		}
	}

	// Sort the render lists
	for (CrRenderList& renderList : m_renderLists)
	{
		renderList.Sort();
	}
}

void CrRenderWorld::BeginRendering(const CrIntrusivePtr<CrCPUStackAllocator>& renderingStream)
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

void CrRenderWorld::SetMouseSelectionEnabled(bool enable, const CrRectangle& boundingRectangle)
{
	m_computeMouseSelection = enable;
	m_mouseSelectionBoundingRectangle = boundingRectangle;
}

bool CrRenderWorld::GetMouseSelectionEnabled() const
{
	return m_computeMouseSelection;
}

void CrRenderList::Clear()
{
	m_renderPackets.clear();
}

void CrRenderList::Sort()
{
	CrQuicksort(m_renderPackets.begin(), m_renderPackets.end());
}
