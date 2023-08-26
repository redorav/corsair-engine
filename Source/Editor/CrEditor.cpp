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

void CrEditor::Initialize()
{
	CrRenderMeshHandle dummyDebugMesh = CrShapeBuilder::CreateSphere({ 12 });
	
	CrMaterialHandle basicMaterial = CrMaterialHandle(new CrMaterial());
	basicMaterial->m_shaders[CrMaterialShaderVariant::Forward] = CrBuiltinPipelines::BasicUbershaderForward.get()->GetShader();
	basicMaterial->m_shaders[CrMaterialShaderVariant::GBuffer] = CrBuiltinPipelines::BasicUbershaderGBuffer.get()->GetShader();
	basicMaterial->m_shaders[CrMaterialShaderVariant::Debug] = CrBuiltinPipelines::BasicUbershaderDebug.get()->GetShader();
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

	float3 currentForward = camera->GetForwardVector();
	float3 currentRight = camera->GetRightVector();

	bool isEscapeClicked = keyboardState.keyHeld[KeyboardKey::Escape];
	//bool isLeftShiftClicked = keyboardState.keyHeld[KeyboardKey::LeftShift];

	float translationSpeed = 5.0f;

	if (keyboardState.keyHeld[KeyboardKey::LeftShift])
	{
		translationSpeed *= 10.0f;
	}

	if (keyboardState.keyHeld[KeyboardKey::A] || gamepadState.axes[GamepadAxis::LeftX] < 0.0f)
	{
		camera->Translate(currentRight * -translationSpeed * frameDelta);
	}

	if (keyboardState.keyHeld[KeyboardKey::D] || gamepadState.axes[GamepadAxis::LeftX] > 0.0f)
	{
		camera->Translate(currentRight * translationSpeed * frameDelta);
	}

	if (keyboardState.keyHeld[KeyboardKey::W] || gamepadState.axes[GamepadAxis::LeftY] > 0.0f)
	{
		camera->Translate(currentForward * translationSpeed * frameDelta);
	}

	if (keyboardState.keyHeld[KeyboardKey::S] || gamepadState.axes[GamepadAxis::LeftY] < 0.0f)
	{
		camera->Translate(currentForward * -translationSpeed * frameDelta);
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
			float3 position = ComputeSelectionPosition();
			m_cameraFocusPosition = position;
			float3 finalCameraPosition = position - camera->GetForwardVector() * 3.0f;
			camera->SetPosition(finalCameraPosition);
		}
	}

	if (keyboardState.keyHeld[KeyboardKey::Alt] && mouseState.buttonHeld[MouseButton::Left])
	{
		float3 pivot = m_cameraFocusPosition;
		float3 up = float3(0.0f, 1.0f, 0.0f);
		float angle = (float)mouseState.relativePosition.x;
		camera->RotateAround(pivot, up, angle);
		camera->LookAtPosition(pivot, up);
	}

	if (gamepadState.axes[GamepadAxis::RightX] > 0.0f)
	{
		camera->Rotate(float3(0.0f, 2.0f, 0.0f) * frameDelta);
	}

	if (gamepadState.axes[GamepadAxis::RightX] < 0.0f)
	{
		camera->Rotate(float3(0.0f, -2.0f, 0.0f) * frameDelta);
	}

	if (mouseState.buttonHeld[MouseButton::Right])
	{
		camera->Rotate(float3(mouseState.relativePosition.y, mouseState.relativePosition.x, 0.0f) * frameDelta);
	}

	camera->Update();

	if (isEscapeClicked)
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
						
						if (m_manipulator->xAxis.GetId() == instanceId)
						{
							m_selectedAxis = CrEditorAxis::AxisX;
						}
						else if (m_manipulator->yAxis.GetId() == instanceId)
						{
							m_selectedAxis = CrEditorAxis::AxisY;
						}
						else if (m_manipulator->zAxis.GetId() == instanceId)
						{
							m_selectedAxis = CrEditorAxis::AxisZ;
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
			CrLog("Manipulator held");

			//int diffX = mouseState.position.x - m_manipulatorInitialMousePosition.x;
			//int diffY = cachedInput.mouseState.position.y - initialMousePosition.y;

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

	bool requestMouseSelection = mouseState.buttonPressed[MouseButton::Left] || mouseState.buttonHeld[MouseButton::Left];

	CrRectangle mouseRectangle(mouseState.position.x, mouseState.position.y, 1, 1);
	m_renderWorld->SetMouseSelectionEnabled(requestMouseSelection, mouseRectangle);
}

void CrEditor::SpawnManipulator(const float4x4& initialTransform)
{
	if (!m_manipulator)
	{
		m_manipulator = CrUniquePtr(new CrManipulator());

		float4 red(1.0f, 0.0f, 0.0f, 1.0f);
		float4 green(0.0f, 1.0f, 0.0f, 1.0f);
		float4 blue(0.0f, 0.0f, 1.0f, 1.0f);

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

		// We grab the shaders from the builtin pipelines, in the knowledge that new pipeline states will be created using whatever
		// the ubershader actually needs in terms of vertex format and render target formats
		CrMaterialHandle basicMaterial = CrMaterialHandle(new CrMaterial());
		basicMaterial->m_shaders[CrMaterialShaderVariant::Forward] = CrBuiltinPipelines::BasicUbershaderForward.get()->GetShader();
		basicMaterial->m_shaders[CrMaterialShaderVariant::GBuffer] = CrBuiltinPipelines::BasicUbershaderGBuffer.get()->GetShader();
		basicMaterial->m_shaders[CrMaterialShaderVariant::Debug] = CrBuiltinPipelines::BasicUbershaderDebug.get()->GetShader();

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

		CrRenderModelHandle xAxisRenderModel = CrRenderModelHandle(new CrRenderModel(xAxisDescriptor));
		CrRenderModelHandle yAxisRenderModel = CrRenderModelHandle(new CrRenderModel(yAxisDescriptor));
		CrRenderModelHandle zAxisRenderModel = CrRenderModelHandle(new CrRenderModel(zAxisDescriptor));

		m_manipulator->xAxis = m_renderWorld->CreateModelInstance();
		m_manipulator->yAxis = m_renderWorld->CreateModelInstance();
		m_manipulator->zAxis = m_renderWorld->CreateModelInstance();

		m_renderWorld->SetRenderModel(m_manipulator->xAxis.GetId(), xAxisRenderModel);
		m_renderWorld->SetRenderModel(m_manipulator->yAxis.GetId(), yAxisRenderModel);
		m_renderWorld->SetRenderModel(m_manipulator->zAxis.GetId(), zAxisRenderModel);

		m_renderWorld->SetConstantSize(m_manipulator->xAxis.GetId(), true);
		m_renderWorld->SetConstantSize(m_manipulator->yAxis.GetId(), true);
		m_renderWorld->SetConstantSize(m_manipulator->zAxis.GetId(), true);

		m_renderWorld->SetEditorInstance(m_manipulator->xAxis.GetId());
		m_renderWorld->SetEditorInstance(m_manipulator->yAxis.GetId());
		m_renderWorld->SetEditorInstance(m_manipulator->zAxis.GetId());
	}

	float4x4 transform = mul(float4x4::scale(0.2f), initialTransform);

	m_manipulator->transformMtx = transform;

	m_renderWorld->SetTransform(m_manipulator->xAxis.GetId(), transform);
	m_renderWorld->SetTransform(m_manipulator->yAxis.GetId(), transform);
	m_renderWorld->SetTransform(m_manipulator->zAxis.GetId(), transform);
}

void CrEditor::RemoveManipulator()
{
	if (m_manipulator)
	{
		m_renderWorld->DestroyModelInstance(m_manipulator->xAxis.GetId());
		m_renderWorld->DestroyModelInstance(m_manipulator->yAxis.GetId());
		m_renderWorld->DestroyModelInstance(m_manipulator->zAxis.GetId());
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

// Assume ray is normalized
float2 ClosestPointToRay(float2 point, float2 rayOrigin, float2 ray, float& t)
{
	float2 originToPoint = point - rayOrigin;
	t = dot(originToPoint, ray) / dot(ray, ray);
	return rayOrigin + t * ray;
}

void CrEditor::TranslateManipulator(const MouseState& mouseState)
{
	float3 closestPointViewRayToAxis = ComputeClosestPointMouseToManipulator(mouseState);

	float3 translationDelta = closestPointViewRayToAxis - m_manipulatorInitialClosestPoint;

	float4x4 transform = m_manipulatorInitialTransform;
	transform[3].xyz += translationDelta;

	m_manipulator->transformMtx = transform;
	m_renderWorld->SetTransform(m_manipulator->xAxis.GetId(), transform);
	m_renderWorld->SetTransform(m_manipulator->yAxis.GetId(), transform);
	m_renderWorld->SetTransform(m_manipulator->zAxis.GetId(), transform);

	// Translate selected entities
	for (const auto& selectedInstanceData : m_selectedInstances)
	{
		const SelectedInstanceState& selectionData = selectedInstanceData.second;
		float3 newPosition = selectionData.initialTransform[3].xyz + translationDelta;
		m_renderWorld->SetPosition(selectionData.modelInstanceId, newPosition);
	}
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
	m_selectedInstances.insert({ instanceId.id, state });
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

float3 CrEditor::ComputeClosestPointMouseToManipulator(const MouseState& mouseState)
{
	// Cast ray from mouse to screen using the camera
	const CrCameraHandle& camera = m_renderWorld->GetCamera();

	//------------------------
	// Calculate current mouse
	//------------------------
	 
	float2 mousePixel = float2(mouseState.position.x, mouseState.position.y) + 0.5f;
	float2 mouseNormalized = mousePixel / float2(camera->GetResolutionWidth(), camera->GetResolutionHeight());
	float2 mouseNdc = mouseNormalized * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
	float2 mouseView = mouseNdc * float2(camera->GetNearPlaneWidth() / 2.0f, camera->GetNearPlaneHeight() / 2.0f);

	//---------------
	// Calculate axis
	//---------------

	// Direction of the manipulator in world space
	float3 manipulatorDirectionWorld;
	
	if (m_selectedAxis == CrEditorAxis::AxisX)
	{
		manipulatorDirectionWorld = normalize(m_manipulatorInitialTransform[0].xyz);
	}
	else if (m_selectedAxis == CrEditorAxis::AxisY)
	{
		manipulatorDirectionWorld = normalize(m_manipulatorInitialTransform[1].xyz);
	}
	else if (m_selectedAxis == CrEditorAxis::AxisZ)
	{
		manipulatorDirectionWorld = normalize(m_manipulatorInitialTransform[2].xyz);
	}

	// Manipulator start position in world space
	float3 manipulatorStartWorld = m_manipulatorInitialTransform[3].xyz;

	// Manipulator end position in world space
	float3 manipulatorEndWorld = manipulatorStartWorld + 10.0f * manipulatorDirectionWorld;

	// Manipulator start and end positions in view space
	float4 manipulatorStartView = mul(float4(manipulatorStartWorld, 1.0f), camera->GetWorld2ViewMatrix());
	float4 manipulatorStartNdc = mul(manipulatorStartView, camera->GetView2ProjectionMatrix());
	manipulatorStartNdc /= manipulatorStartNdc.wwww;

	float4 manipulatorEndView = mul(float4(manipulatorEndWorld, 1.0f), camera->GetWorld2ViewMatrix());
	float4 manipulatorEndNdc = mul(manipulatorEndView, camera->GetView2ProjectionMatrix());
	manipulatorEndNdc /= manipulatorEndNdc.wwww;

	float4 uvScale(0.5f, -0.5f, 1.0f, 1.0f);
	float4 uvBias(0.5f, 0.5f, 0.0f, 0.0f);
	
	float4 manipulatorStartUV = manipulatorStartNdc * uvScale + uvBias;
	float4 manipulatorStartPixel = manipulatorStartUV * float4(camera->GetResolutionWidth(), camera->GetResolutionHeight(), 1.0f, 1.0f);

	float4 manipulatorEndUV = manipulatorEndNdc * uvScale + uvBias;
	float4 manipulatorEndPixel = manipulatorEndUV * float4(camera->GetResolutionWidth(), camera->GetResolutionHeight(), 1.0f, 1.0f);

	// Bring the point onto the near plane by similar triangles
	//manipulatorStartView.xy = manipulatorStartView.xy * nearPlane / manipulatorStartView.z;

	//manipulatorEndView.xy = manipulatorEndView.xy * nearPlane / manipulatorEndView.z;

	// Manipulator direction in NDC space
	float2 manipulatorAxisPixel = (manipulatorEndPixel.xy - manipulatorStartPixel.xy);

	float t;
	float2 closestPointMouseToRay = ClosestPointToRay(mousePixel, manipulatorStartPixel.xy, manipulatorAxisPixel, t);

	float3 interpolatedWorldPositionInvZ = lerp(manipulatorStartWorld / manipulatorStartView.z, manipulatorEndWorld / manipulatorEndView.z, t);

	float1 interpolatedInvZ = lerp(1.0f / manipulatorStartView.z, 1.0f / manipulatorEndView.z, t);

	float3 interpolatedWorldPosition = interpolatedWorldPositionInvZ / interpolatedInvZ;

	return interpolatedWorldPosition;
}
