#include "Editor/CrEditor.h"

#include "Math/CrMath.h"

#include "Core/Input/CrInputManager.h"
#include "Core/CrFrameTime.h"

#include "Rendering/RenderWorld/CrRenderWorld.h"
#include "Rendering/CrCamera.h"
#include "Rendering/CrBuiltinPipeline.h"
#include "Rendering/CrShapeBuilder.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/ICrShader.h"

#include "Math/CrHlslppQuaternion.h"
#include "Math/CrHlslppMatrixFloat.h"

void CrEditor::Initialize()
{
	CrRenderMeshHandle dummyDebugMesh = CrShapeBuilder::CreateSphere({ 12 });
	
	CrMaterialHandle basicMaterial = CrMaterialHandle(new CrMaterial());
	basicMaterial->m_shaders[CrMaterialShaderVariant::Forward] = BuiltinPipelines->BasicUbershaderForward->GetShader();
	basicMaterial->m_shaders[CrMaterialShaderVariant::GBuffer] = BuiltinPipelines->BasicUbershaderGBuffer->GetShader();
	basicMaterial->m_shaders[CrMaterialShaderVariant::Debug]   = BuiltinPipelines->BasicUbershaderDebug->GetShader();

	m_cameraState.defaultFocusDistance = 4.0f;
	m_cameraState.focusDistance = m_cameraState.defaultFocusDistance;
}

void CrEditor::SetRenderWorld(const CrRenderWorldHandle& renderWorld)
{
	m_renderWorld = renderWorld;
}

void CrEditor::AddSelectionState(const SelectionState& selectionState)
{
	m_selectionStateQueue.push_back(selectionState);
}

void CrEditor::Update()
{
	const CrCameraHandle& camera = m_renderWorld->GetCamera();

	float frameDelta = (float)CrFrameTime::GetFrameDelta().AsSeconds();

	const MouseState& mouseState = CrInput.GetMouseState();
	const KeyboardState& keyboardState = CrInput.GetKeyboardState();
	const GamepadState& gamepadState = CrInput.GetGamepadState(0);

	bool isEscapeHeld = keyboardState.keyHeld[KeyboardKey::Escape];
	//bool isLeftShiftClicked = keyboardState.keyHeld[KeyboardKey::LeftShift];
	bool isLeftAltHeld = keyboardState.keyHeld[KeyboardKey::Alt];
	//bool isLeftCtrlHeld = keyboardState.keyHeld[KeyboardKey::LeftCtrl];
	bool isLeftShiftHeld = keyboardState.keyHeld[KeyboardKey::LeftShift];

	bool isFPSCamera = mouseState.buttonHeld[MouseButton::Right];
	bool isOrbitCamera = isLeftAltHeld && mouseState.buttonHeld[MouseButton::Left];

	//---------
	// Rotation
	//---------

	float mouseSensitivity = 0.005f;
	float mouseWheelSensitivity = 0.5f;

	if (gamepadState.axes[GamepadAxis::RightX] > 0.0f)
	{
		m_cameraState.yaw += 2.0f * frameDelta;
	}

	if (gamepadState.axes[GamepadAxis::RightX] < 0.0f)
	{
		m_cameraState.yaw += -2.0f * frameDelta;
	}

	if (gamepadState.axes[GamepadAxis::RightY] > 0.0f)
	{
		m_cameraState.pitch += -2.0f * frameDelta;
	}

	if (gamepadState.axes[GamepadAxis::RightY] < 0.0f)
	{
		m_cameraState.pitch += 2.0f * frameDelta;
	}

	if (isFPSCamera || isOrbitCamera)
	{
		m_cameraState.pitch += (float)mouseState.relativePosition.y * mouseSensitivity;
		m_cameraState.yaw += (float)mouseState.relativePosition.x * mouseSensitivity;
	}

	// Restrict angles to 2pi to avoid numerical instability at large values
	// Compute rotation from absolute angles relative to the world origin
	// This avoids relative camera movement that can drift and avoids Y-axis nonsense
	float3x3 xMtx = float3x3::rotation_x(m_cameraState.pitch);
	float3x3 yMtx = float3x3::rotation_y(m_cameraState.yaw);
	float3x3 finalMtx = mul(xMtx, yMtx);

	camera->SetCameraRotationVectors(finalMtx[2], finalMtx[0], finalMtx[1]);

	const float3 forwardVector = camera->GetForwardVector();
	const float3 rightVector = camera->GetRightVector();
	const float3 upVector = camera->GetUpVector();

	//------------
	// Translation
	//------------

	float translationSpeed = 5.0f;

	if (isLeftShiftHeld)
	{
		translationSpeed *= 10.0f;
	}

	if (keyboardState.keyHeld[KeyboardKey::A] || gamepadState.axes[GamepadAxis::LeftX] < 0.0f)
	{
		camera->Translate(rightVector * -translationSpeed * frameDelta);
	}

	if (keyboardState.keyHeld[KeyboardKey::D] || gamepadState.axes[GamepadAxis::LeftX] > 0.0f)
	{
		camera->Translate(rightVector * translationSpeed * frameDelta);
	}

	if (keyboardState.keyHeld[KeyboardKey::W] || gamepadState.axes[GamepadAxis::LeftY] > 0.0f)
	{
		camera->Translate(forwardVector * translationSpeed * frameDelta);
	}

	if (keyboardState.keyHeld[KeyboardKey::S] || gamepadState.axes[GamepadAxis::LeftY] < 0.0f)
	{
		camera->Translate(forwardVector * -translationSpeed * frameDelta);
	}

	if (keyboardState.keyHeld[KeyboardKey::Q] || gamepadState.axes[GamepadAxis::LeftTrigger] > 0.0f)
	{
		camera->Translate(float3(0.0f, -translationSpeed, 0.0f) * frameDelta);
	}

	if (keyboardState.keyHeld[KeyboardKey::E] || gamepadState.axes[GamepadAxis::RightTrigger] > 0.0f)
	{
		camera->Translate(float3(0.0f, translationSpeed, 0.0f) * frameDelta);
	}

	if (keyboardState.keyHeld[KeyboardKey::F])
	{
		if (IsAnyInstanceSelected())
		{
			m_cameraState.focusDistance = m_cameraState.defaultFocusDistance;
			m_cameraState.focusPosition = ComputeSelectionPosition();
			camera->SetPosition(m_cameraState.focusPosition - forwardVector * m_cameraState.focusDistance);
		}
	}

	if (mouseState.mouseWheel.y != 0)
	{
		float translation = mouseState.mouseWheel.y * mouseWheelSensitivity;
		camera->Translate(forwardVector * translation);
		m_cameraState.focusDistance = distance(camera->GetPosition(), m_cameraState.focusPosition);
	}

	//-----------------
	// Orbit and Strafe
	//-----------------

	if (isLeftAltHeld)
	{
		if (mouseState.buttonHeld[MouseButton::Left])
		{
			// If we're orbiting the focal point, make sure we continually point towards it
			camera->SetPosition(m_cameraState.focusPosition - forwardVector * m_cameraState.focusDistance);
		}
		else if(mouseState.buttonHeld[MouseButton::Middle])
		{
			float rightTranslation = (float)-mouseState.relativePosition.x * mouseSensitivity;
			float upTranslation = (float)mouseState.relativePosition.y * mouseSensitivity;

			camera->SetPosition(camera->GetPosition() + rightVector * rightTranslation + upVector * upTranslation);
		}

		// Finally update the focus position
		m_cameraState.focusPosition = camera->GetPosition() + forwardVector * m_cameraState.focusDistance;
	}

	//--------------------------
	// Update Matrices to Render
	//--------------------------

	camera->UpdateMatrices();

	if (isEscapeHeld)
	{
		ClearSelectedInstances();
		RemoveManipulator();
	}

	for (size_t i = 0; i < m_selectionStateQueue.size(); ++i)
	{
		const SelectionState& selectionState = m_selectionStateQueue[i];

		// If we have a valid entity, we can start seeing how to react
		if (selectionState.modelInstanceId != 0xffffffff)
		{
			if (selectionState.modelInstanceId != 65535) // TODO Why this double check?
			{
				CrModelInstanceId instanceId = CrModelInstanceId(selectionState.modelInstanceId);

				bool isEditorInstance = m_renderWorld->GetIsEditorInstance(instanceId);

				if (selectionState.mouseState.buttonPressed[MouseButton::Left])
				{
					if (isEditorInstance)
					{
						m_manipulatorSelected = true;
						
						if (instanceId == m_manipulator->xAxis.GetId())
						{
							m_selectedAxis = CrEditorAxis::AxisX;
						}
						else if (instanceId == m_manipulator->yAxis.GetId())
						{
							m_selectedAxis = CrEditorAxis::AxisY;
						}
						else if (instanceId == m_manipulator->zAxis.GetId())
						{
							m_selectedAxis = CrEditorAxis::AxisZ;
						}
						else if (instanceId == m_manipulator->xzPlane.GetId())
						{
							m_selectedAxis = CrEditorAxis::PlaneXZ;
						}
						else if (instanceId == m_manipulator->xyPlane.GetId())
						{
							m_selectedAxis = CrEditorAxis::PlaneXY;
						}
						else if (instanceId == m_manipulator->yzPlane.GetId())
						{
							m_selectedAxis = CrEditorAxis::PlaneYZ;
						}

						CrAssertMsg(m_selectedAxis != CrEditorAxis::None, "Incorrect axis selected");

						m_manipulatorInitialMouseState = selectionState.mouseState;
						m_manipulatorInitialTransform = m_manipulator->transformMtx;
						m_manipulatorInitialClosestPoint = ComputeClosestPointMouseToManipulator(selectionState.mouseState);
					}
					else
					{
						if (selectionState.keyboardState.keyHeld[KeyboardKey::LeftShift])
						{
							ToggleSelected(instanceId);

							// TODO when deselecting, don't spawn the manipulator there
							SpawnManipulator(m_renderWorld->GetTransform(instanceId));
						}
						else
						{
							SetSelected(CrModelInstanceId(selectionState.modelInstanceId));

							SpawnManipulator(m_renderWorld->GetTransform(instanceId));
						}
					}
				}
				else if (selectionState.mouseState.buttonHeld[MouseButton::Left])
				{
					
				}
				else if (selectionState.mouseState.buttonClicked[MouseButton::Left])
				{
					
				}
			}
		}
	}

	m_selectionStateQueue.clear();

	if (mouseState.buttonHeld[MouseButton::Left])
	{
		if (m_manipulatorSelected)
		{
			TranslateManipulator(mouseState);
		}
	}
	else if (mouseState.buttonClicked[MouseButton::Left])
	{
		for (auto& selectedInstanceData : m_selectedInstances)
		{
			SelectedInstanceState& selectionData = selectedInstanceData.second;
			selectionData.initialTransform = m_renderWorld->GetTransform(selectionData.modelInstanceId);
		}

		m_manipulatorSelected = false;
	}

	if (keyboardState.keyPressed[KeyboardKey::Delete])
	{
		for (const auto& selectedInstance : m_selectedInstances)
		{
			m_renderWorld->DestroyModelInstance(selectedInstance.second.modelInstanceId);
		}

		m_selectedInstances.clear();

		RemoveManipulator();
	}

	bool requestMouseSelection = 
		(mouseState.buttonPressed[MouseButton::Left] || mouseState.buttonHeld[MouseButton::Left])
		&& !isLeftAltHeld; // We use left alt for navigation, so we want to avoid clicking on things

	CrRectangle mouseRectangle(mouseState.position.x, mouseState.position.y, 1, 1);
	m_renderWorld->SetMouseSelectionEnabled(requestMouseSelection, mouseRectangle);
}

void CrEditor::SpawnManipulator(const float4x4& initialTransform)
{
	if (!m_manipulator)
	{
		m_manipulator = CrUniquePtr<CrManipulator>(new CrManipulator());

		// These colors should be in linear space as we write the manipulator to the HDR buffer
		float4 red(1.0f, 0.07f, 0.07f, 1.0f);
		float4 green(0.07f, 1.0f, 0.07f, 1.0f);
		float4 blue(0.07f, 0.07f, 1.0f, 1.0f);

		float4 transparentRed(1.0f, 0.07f, 0.07f, 0.5f);
		float4 transparentGreen(0.07f, 1.0f, 0.07f, 0.5f);
		float4 transparentBlue(0.07f, 0.07f, 1.0f, 0.5f);

		float4x4 rotXMtx = float4x4::rotation_x(CrMath::Pi / 2.0f);
		float4x4 rotZMtx = float4x4::rotation_z(-CrMath::Pi / 2.0f);

		float cylinderWidth = 0.02f;
		float coneWidth = cylinderWidth * 3.0f;
		float coneHeight = coneWidth * 2.0f;
		float4x4 scaleCylinderMtx = float4x4::scale(cylinderWidth, 0.5f, cylinderWidth);
		float4x4 scaleConeMtx = float4x4::scale(coneWidth, coneHeight, coneWidth);

		float4x4 xAxisCylinderMtx = mul(scaleCylinderMtx, rotZMtx);
		xAxisCylinderMtx[3] = float4(0.5f, 0.0f, 0.0f, 1.0f);
		CrRenderMeshHandle xAxisCylinder = CrShapeBuilder::CreateCylinder({ 12, 0, xAxisCylinderMtx, red });

		float4x4 xAxisConeMtx = mul(scaleConeMtx, rotZMtx);
		xAxisConeMtx[3] = float4(1.0f, 0.0f, 0.0f, 1.0f);
		CrRenderMeshHandle xAxisCone = CrShapeBuilder::CreateCone({ 12, 0, xAxisConeMtx, red });

		float4x4 yAxisCylinderMtx = scaleCylinderMtx;
		yAxisCylinderMtx[3] = float4(0.0f, 0.5f, 0.0f, 1.0f);
		CrRenderMeshHandle yAxisCylinder = CrShapeBuilder::CreateCylinder({ 12, 0, yAxisCylinderMtx, green });

		float4x4 yAxisConeMtx = scaleConeMtx;
		yAxisConeMtx[3] = float4(0.0f, 1.0f, 0.0f, 1.0f);
		CrRenderMeshHandle yAxisCone = CrShapeBuilder::CreateCone({ 12, 0, yAxisConeMtx, green });

		float4x4 zAxisCylinderMtx = mul(scaleCylinderMtx, rotXMtx);
		zAxisCylinderMtx[3] = float4(0.0f, 0.0f, 0.5f, 1.0f);
		CrRenderMeshHandle zAxisCylinder = CrShapeBuilder::CreateCylinder({ 12, 0, zAxisCylinderMtx, blue });

		float4x4 zAxisConeMtx = mul(scaleConeMtx, rotXMtx);
		zAxisConeMtx[3] = float4(0.0f, 0.0f, 1.0f, 1.0f);
		CrRenderMeshHandle zAxisCone = CrShapeBuilder::CreateCone({ 12, 0, zAxisConeMtx, blue });

		// We want the plane to be a quarter of the size of the axis, so we divide by 2.0 (to get to the unit)
		// and then divide by 4 (to get to a quarter)
		float4x4 xzPlaneMtx = float4x4::scale(0.125f);
		xzPlaneMtx[3] = float4(0.125f, 0.0f, 0.125f, 1.0f);
		CrRenderMeshHandle xzPlaneQuad = CrShapeBuilder::CreateQuad({ 0, 0, xzPlaneMtx, transparentGreen });
		xzPlaneQuad->SetIsDoubleSided(true);

		float4x4 xyPlaneMtx = mul(float4x4::scale(0.125f), float4x4::rotation_x(1.570796f));
		xyPlaneMtx[3] = float4(0.125f, 0.125f, 0.0f, 1.0f);
		CrRenderMeshHandle xyPlaneQuad = CrShapeBuilder::CreateQuad({ 0, 0, xyPlaneMtx, transparentBlue });
		xyPlaneQuad->SetIsDoubleSided(true);

		float4x4 yzPlaneMtx = mul(float4x4::scale(0.125f), float4x4::rotation_z(1.570796f));
		yzPlaneMtx[3] = float4(0.0f, 0.125f, 0.125f, 1.0f);
		CrRenderMeshHandle yzPlaneQuad = CrShapeBuilder::CreateQuad({ 0, 0, yzPlaneMtx, transparentRed });
		yzPlaneQuad->SetIsDoubleSided(true);

		// We grab the shaders from the builtin pipelines, in the knowledge that new pipeline states will be created using whatever
		// the ubershader actually needs in terms of vertex format and render target formats
		// We don't have an opaque shader here. We probably don't need them for editor meshes
		CrMaterialHandle basicMaterial = CrMaterialHandle(new CrMaterial());
		basicMaterial->m_shaders[CrMaterialShaderVariant::Forward] = BuiltinPipelines->BasicUbershaderForward->GetShader();
		basicMaterial->m_shaders[CrMaterialShaderVariant::Debug] = BuiltinPipelines->BasicUbershaderDebug->GetShader();

		CrRenderModelDescriptor xAxisDescriptor;
		xAxisDescriptor.AddMaterial(basicMaterial);
		xAxisDescriptor.AddRenderMesh(xAxisCylinder, 0);
		xAxisDescriptor.AddRenderMesh(xAxisCone, 0);

		CrRenderModelDescriptor yAxisDescriptor;
		yAxisDescriptor.AddMaterial(basicMaterial);
		yAxisDescriptor.AddRenderMesh(yAxisCylinder, 0);
		yAxisDescriptor.AddRenderMesh(yAxisCone, 0);

		CrRenderModelDescriptor zAxisDescriptor;
		zAxisDescriptor.AddMaterial(basicMaterial);
		zAxisDescriptor.AddRenderMesh(zAxisCylinder, 0);
		zAxisDescriptor.AddRenderMesh(zAxisCone, 0);

		CrRenderModelDescriptor xzPlaneDescriptor;
		xzPlaneDescriptor.AddMaterial(basicMaterial);
		xzPlaneDescriptor.AddRenderMesh(xzPlaneQuad, 0);

		CrRenderModelDescriptor xyPlaneDescriptor;
		xyPlaneDescriptor.AddMaterial(basicMaterial);
		xyPlaneDescriptor.AddRenderMesh(xyPlaneQuad, 0);

		CrRenderModelDescriptor yzPlaneDescriptor;
		yzPlaneDescriptor.AddMaterial(basicMaterial);
		yzPlaneDescriptor.AddRenderMesh(yzPlaneQuad, 0);

		CrRenderModelHandle xAxisRenderModel = CrRenderModelHandle(new CrRenderModel(xAxisDescriptor));
		CrRenderModelHandle yAxisRenderModel = CrRenderModelHandle(new CrRenderModel(yAxisDescriptor));
		CrRenderModelHandle zAxisRenderModel = CrRenderModelHandle(new CrRenderModel(zAxisDescriptor));
		CrRenderModelHandle xzPlaneRenderModel = CrRenderModelHandle(new CrRenderModel(xzPlaneDescriptor));
		CrRenderModelHandle xyPlaneRenderModel = CrRenderModelHandle(new CrRenderModel(xyPlaneDescriptor));
		CrRenderModelHandle yzPlaneRenderModel = CrRenderModelHandle(new CrRenderModel(yzPlaneDescriptor));

		m_manipulator->xAxis = m_renderWorld->CreateModelInstance();
		m_manipulator->yAxis = m_renderWorld->CreateModelInstance();
		m_manipulator->zAxis = m_renderWorld->CreateModelInstance();
		m_manipulator->xzPlane = m_renderWorld->CreateModelInstance();
		m_manipulator->xyPlane = m_renderWorld->CreateModelInstance();
		m_manipulator->yzPlane = m_renderWorld->CreateModelInstance();

		m_renderWorld->SetRenderModel(m_manipulator->xAxis.GetId(), xAxisRenderModel);
		m_renderWorld->SetRenderModel(m_manipulator->yAxis.GetId(), yAxisRenderModel);
		m_renderWorld->SetRenderModel(m_manipulator->zAxis.GetId(), zAxisRenderModel);
		m_renderWorld->SetRenderModel(m_manipulator->xzPlane.GetId(), xzPlaneRenderModel);
		m_renderWorld->SetRenderModel(m_manipulator->xyPlane.GetId(), xyPlaneRenderModel);
		m_renderWorld->SetRenderModel(m_manipulator->yzPlane.GetId(), yzPlaneRenderModel);

		//m_renderWorld->SetMaterial();

		m_renderWorld->SetConstantSize(m_manipulator->xAxis.GetId(), true);
		m_renderWorld->SetConstantSize(m_manipulator->yAxis.GetId(), true);
		m_renderWorld->SetConstantSize(m_manipulator->zAxis.GetId(), true);
		m_renderWorld->SetConstantSize(m_manipulator->xzPlane.GetId(), true);
		m_renderWorld->SetConstantSize(m_manipulator->xyPlane.GetId(), true);
		m_renderWorld->SetConstantSize(m_manipulator->yzPlane.GetId(), true);

		m_renderWorld->SetEditorInstance(m_manipulator->xAxis.GetId());
		m_renderWorld->SetEditorInstance(m_manipulator->yAxis.GetId());
		m_renderWorld->SetEditorInstance(m_manipulator->zAxis.GetId());
		m_renderWorld->SetEditorInstance(m_manipulator->xzPlane.GetId());
		m_renderWorld->SetEditorInstance(m_manipulator->xyPlane.GetId());
		m_renderWorld->SetEditorInstance(m_manipulator->yzPlane.GetId());
	}

	float4x4 transform = mul(float4x4::scale(0.2f), initialTransform);

	SetManipulatorTransform(transform);
}

void CrEditor::RemoveManipulator()
{
	if (m_manipulator)
	{
		m_renderWorld->DestroyModelInstance(m_manipulator->xAxis.GetId());
		m_renderWorld->DestroyModelInstance(m_manipulator->yAxis.GetId());
		m_renderWorld->DestroyModelInstance(m_manipulator->zAxis.GetId());
		m_renderWorld->DestroyModelInstance(m_manipulator->xzPlane.GetId());
		m_renderWorld->DestroyModelInstance(m_manipulator->xyPlane.GetId());
		m_renderWorld->DestroyModelInstance(m_manipulator->yzPlane.GetId());
		m_manipulator = nullptr;
	}
}

// https://palitri.com/vault/stuff/maths/Rays%20closest%20point.pdf
// Return point on ray1 that is closest to ray0
// Assume rays are normalized, therefore AdotA and BdotB are 1.0
float3 ClosestPointRayToRay(float3 pointA, float3 rayA, float3 pointB, float3 rayB)
{
	float3 rayC = pointB - pointA;
	
	float AdotB = dot(rayA, rayB);
	float AdotC = dot(rayA, rayC);
	float BdotC = dot(rayB, rayC);

	float tNumerator = AdotB * AdotC - BdotC;
	float tDenominator = 1.0f - AdotB * AdotB;

	float3 E = pointB + rayB * tNumerator / tDenominator;

	return E;
}

float2 ClosestPointToRay(float2 point, float2 rayOrigin, float2 ray, float& t)
{
	float2 originToPoint = point - rayOrigin;
	t = dot(originToPoint, ray) / dot(ray, ray);
	return rayOrigin + t * ray;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection.html
float3 RayPlaneIntersection(float3 planePoint, float3 planeNormal, float3 position, float3 direction)
{
	float3 positionToPlane = planePoint - position;
	float t = dot(positionToPlane, planeNormal) / dot(direction, planeNormal);
	return position + t * direction;
}

void CrEditor::TranslateManipulator(const MouseState& mouseState)
{
	float3 closestPointViewRayToAxis = ComputeClosestPointMouseToManipulator(mouseState);

	float3 translationDelta = closestPointViewRayToAxis - m_manipulatorInitialClosestPoint;

	float4x4 transform = m_manipulatorInitialTransform;
	transform[3].xyz += translationDelta;

	SetManipulatorTransform(transform);

	// Translate selected entities
	for (const auto& selectedInstanceData : m_selectedInstances)
	{
		const SelectedInstanceState& selectionData = selectedInstanceData.second;
		float3 newPosition = selectionData.initialTransform[3].xyz + translationDelta;
		m_renderWorld->SetPosition(selectionData.modelInstanceId, newPosition);
	}
}

void CrEditor::SetManipulatorTransform(const float4x4& transform)
{
	m_manipulator->transformMtx = transform;
	m_renderWorld->SetTransform(m_manipulator->xAxis.GetId(), transform);
	m_renderWorld->SetTransform(m_manipulator->yAxis.GetId(), transform);
	m_renderWorld->SetTransform(m_manipulator->zAxis.GetId(), transform);
	m_renderWorld->SetTransform(m_manipulator->xzPlane.GetId(), transform);
	m_renderWorld->SetTransform(m_manipulator->xyPlane.GetId(), transform);
	m_renderWorld->SetTransform(m_manipulator->yzPlane.GetId(), transform);
}

void CrEditor::SetSelected(CrModelInstanceId instanceId)
{
	ClearSelectedInstances();
	AddSelected(instanceId);
}

void CrEditor::ToggleSelected(CrModelInstanceId instanceId)
{
	if (!m_renderWorld->GetIsEditorInstance(instanceId))
	{
		if (GetIsSelected(instanceId))
		{
			RemoveSelected(instanceId);
		}
		else
		{
			AddSelected(instanceId);
		}
	}
}

bool CrEditor::GetIsSelected(CrModelInstanceId instanceId)
{
	return m_selectedInstances.find(instanceId.id) != m_selectedInstances.end();
}

void CrEditor::AddSelected(CrModelInstanceId instanceId)
{
	m_renderWorld->SetIsEditorEdgeHighlight(instanceId, true);
	SelectedInstanceState state;
	state.modelInstanceId = instanceId;
	state.initialTransform = m_renderWorld->GetTransform(instanceId);
	m_selectedInstances.insert(instanceId.id, state);
}

void CrEditor::RemoveSelected(CrModelInstanceId instanceId)
{
	m_renderWorld->SetIsEditorEdgeHighlight(instanceId, false);
	m_selectedInstances.erase(instanceId.id);
}

void CrEditor::ClearSelectedInstances()
{
	for (const auto& selectedInstance : m_selectedInstances)
	{
		m_renderWorld->SetIsEditorEdgeHighlight(selectedInstance.second.modelInstanceId, false);
	}

	m_selectedInstances.clear();
}

bool CrEditor::IsAnyInstanceSelected() const
{
	return !m_selectedInstances.empty();
}

float3 CrEditor::ComputeSelectionPosition() const
{
	float4 averagePosition;

	for (const auto& selectedInstance : m_selectedInstances)
	{
		const float4x4& transform = selectedInstance.second.initialTransform;
		averagePosition += transform[3];
	}

	averagePosition /= (float)m_selectedInstances.size();

	return averagePosition.xyz;
}

// Compute closest point in axis to current world position
float3 CrEditor::ComputeClosestPointToAxis(float2 mousePixel, float3 axisPositionWorld, float3 axisDirectionWorld)
{
	const CrCameraHandle& camera = m_renderWorld->GetCamera();

	// Manipulator start position in world space
	float3 axisStartWorld = axisPositionWorld;

	// Manipulator end position in world space
	float3 axisEndWorld = axisStartWorld + axisDirectionWorld;

	// Manipulator start and end positions in view space
	float4 manipulatorStartView = mul(float4(axisStartWorld, 1.0f), camera->GetWorld2ViewMatrix());
	float4 manipulatorStartNdc = mul(manipulatorStartView, camera->GetView2ProjectionMatrix());
	manipulatorStartNdc /= manipulatorStartNdc.wwww;

	float4 manipulatorEndView = mul(float4(axisEndWorld, 1.0f), camera->GetWorld2ViewMatrix());
	float4 manipulatorEndNdc = mul(manipulatorEndView, camera->GetView2ProjectionMatrix());
	manipulatorEndNdc /= manipulatorEndNdc.wwww;

	float4 uvScale(0.5f, -0.5f, 1.0f, 1.0f);
	float4 uvBias(0.5f, 0.5f, 0.0f, 0.0f);

	float4 manipulatorStartUV = manipulatorStartNdc * uvScale + uvBias;
	float4 manipulatorStartPixel = manipulatorStartUV * float4(camera->GetResolutionWidth(), camera->GetResolutionHeight(), 1.0f, 1.0f);

	float4 manipulatorEndUV = manipulatorEndNdc * uvScale + uvBias;
	float4 manipulatorEndPixel = manipulatorEndUV * float4(camera->GetResolutionWidth(), camera->GetResolutionHeight(), 1.0f, 1.0f);

	float4 manipulatorAxisPixel = manipulatorEndPixel - manipulatorStartPixel;

	float t;
	float2 closestPointMouseToRay = ClosestPointToRay(mousePixel, manipulatorStartPixel.xy, manipulatorAxisPixel.xy, t);

	// Perspective-correct interpolation between the world positions. We need this because we found a t in pixel space, and cannot be used directly
	float3 interpolatedWorldPositionInvZ = lerp(axisStartWorld / manipulatorStartView.z, axisEndWorld / manipulatorEndView.z, t);

	float1 interpolatedInvZ = lerp(1.0f / manipulatorStartView.z, 1.0f / manipulatorEndView.z, t);

	float3 interpolatedWorldPosition = interpolatedWorldPositionInvZ / interpolatedInvZ;

	return interpolatedWorldPosition;
}

float3 CrEditor::ComputeClosestPointToPlane(float2 mousePixel, float3 planePositionWorld, float3 planeNormalWorld)
{
	const CrCameraHandle& camera = m_renderWorld->GetCamera();

	float2 mouseNdc = (mousePixel / float2(camera->GetResolutionWidth(), camera->GetResolutionHeight())) * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);

	float3 viewRay = camera->GetViewRay(mouseNdc);

	float3 viewRayWorld = mul(viewRay, camera->GetView2WorldRotation());

	return RayPlaneIntersection(planePositionWorld, planeNormalWorld, camera->GetPosition(), viewRayWorld);
}

float3 CrEditor::ComputeClosestPointMouseToManipulator(const MouseState& mouseState)
{
	float2 mousePixel = float2(mouseState.position.x, mouseState.position.y) + 0.5f;

	// Direction of the manipulator in world space
	float3 manipulatorDirectionWorld;
	float3 closestPointToManipulator;
	
	if (m_selectedAxis == CrEditorAxis::AxisX)
	{
		closestPointToManipulator = ComputeClosestPointToAxis(mousePixel, m_manipulatorInitialTransform[3].xyz, normalize(m_manipulatorInitialTransform[0].xyz));
	}
	else if (m_selectedAxis == CrEditorAxis::AxisY)
	{
		closestPointToManipulator = ComputeClosestPointToAxis(mousePixel, m_manipulatorInitialTransform[3].xyz, normalize(m_manipulatorInitialTransform[1].xyz));
	}
	else if (m_selectedAxis == CrEditorAxis::AxisZ)
	{
		closestPointToManipulator = ComputeClosestPointToAxis(mousePixel, m_manipulatorInitialTransform[3].xyz, normalize(m_manipulatorInitialTransform[2].xyz));
	}
	else if (m_selectedAxis == CrEditorAxis::PlaneXZ)
	{
		closestPointToManipulator = ComputeClosestPointToPlane(mousePixel, m_manipulatorInitialTransform[3].xyz, m_manipulatorInitialTransform[1].xyz);
	}
	else if (m_selectedAxis == CrEditorAxis::PlaneXY)
	{
		closestPointToManipulator = ComputeClosestPointToPlane(mousePixel, m_manipulatorInitialTransform[3].xyz, m_manipulatorInitialTransform[2].xyz);
	}
	else if (m_selectedAxis == CrEditorAxis::PlaneYZ)
	{
		closestPointToManipulator = ComputeClosestPointToPlane(mousePixel, m_manipulatorInitialTransform[3].xyz, m_manipulatorInitialTransform[0].xyz);
	}

	return closestPointToManipulator;
}