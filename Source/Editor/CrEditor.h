#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRendering.h"
#include "Rendering/RenderWorld/CrRenderModelInstance.h"

#include "Math/CrHlslppMatrixFloatType.h"
#include "Math/CrHlslppVectorFloatType.h"
#include "Math/CrHlslppVectorIntType.h"

#include "Core/Input/CrInputManager.h"
#include "Core/SmartPointers/CrIntrusivePtr.h"
#include "Core/SmartPointers/CrUniquePtr.h"
#include "Core/Containers/CrVector.h"
#include "Core/Containers/CrHashMap.h"

// Stores state when we selected or clicked something so that we're able to
// determine the appropriate action from that
struct SelectionState
{
	// Id of the entity we've selected
	uint32_t modelInstanceId = 0xffffffff;

	MouseState mouseState;

	KeyboardState keyboardState;
};

struct InputState
{
	MouseState mouseState;

	KeyboardState keyboardState;

	GamepadState gamepadState;
};

struct CrManipulator
{
	float4x4 transformMtx;

	CrRenderModelInstance xAxis;
	CrRenderModelInstance yAxis;
	CrRenderModelInstance zAxis;
	CrRenderModelInstance xzPlane;
	CrRenderModelInstance xyPlane;
	CrRenderModelInstance yzPlane;
};

// Properties of instances that are currently selected. We use this to be able to transform
// relative to their original state instead of accumulating which can produce drift
struct SelectedInstanceState
{
	CrModelInstanceId modelInstanceId;

	float4x4 initialTransform;
};

struct CrEditorCameraState
{
	float3 focusPosition;
	float focusDistance;
	float defaultFocusDistance;
	float yaw = 0.0f;
	float pitch = 0.0f;
};

namespace CrEditorAxis
{
	enum T
	{
		AxisX,
		AxisY,
		AxisZ,
		PlaneXZ,
		PlaneXY,
		PlaneYZ,
		None
	};
};

class CrOSWindow;

class CrEditor
{
public:

	static void Initialize(const CrIntrusivePtr<CrOSWindow>& mainWindow);

	static void Deinitialize();

	void SetRenderWorld(const CrRenderWorldHandle& renderWorld);

	void AddSelectionState(const SelectionState& selectionState);

	void Update();

private:

	CrEditor(const CrIntrusivePtr<CrOSWindow>& mainWindow);

	void SpawnManipulator(const float4x4& initialTransform);

	void RemoveManipulator();

	void TranslateManipulator(const MouseState& mouseState);

	void SetManipulatorTransform(const float4x4& transform);

	// Selection functionality

	void SetSelected(CrModelInstanceId instanceId);

	void ToggleSelected(CrModelInstanceId instanceId);

	bool GetIsSelected(CrModelInstanceId instanceId);

	void AddSelected(CrModelInstanceId instanceId);

	void RemoveSelected(CrModelInstanceId instanceId);

	void ClearSelectedInstances();

	bool IsAnyInstanceSelected() const;

	float3 ComputeSelectionPosition() const;

	float3 ComputeClosestPointToAxis(float2 mousePixel, float3 axisPositionWorld, float3 axisDirectionWorld);

	float3 ComputeClosestPointToPlane(float2 mousePixel, float3 planePosition, float3 planeNormal);

	float3 ComputeClosestPointMouseToManipulator(const MouseState& mouseState);

	CrRenderWorldHandle m_renderWorld;
	
	// Manipulator and selection related variables

	CrVector<SelectionState> m_selectionStateQueue;

	CrHashMap<CrModelInstanceId::type, SelectedInstanceState> m_selectedInstances;

	// Pointer to a currently spawned manipulator
	CrUniquePtr<CrManipulator> m_manipulator;

	// Whether the manipulator is selected (e.g. we started dragging)
	bool m_manipulatorSelected = false;

	// Which axis we have selected when dragging the manipulator
	CrEditorAxis::T m_selectedAxis = CrEditorAxis::None;

	// Position of the mouse when we started dragging
	MouseState m_manipulatorInitialMouseState;

	// Transform of the manipulator when we started dragging it
	float4x4 m_manipulatorInitialTransform;

	// Closest point from ray to axis when we started dragging
	float3 m_manipulatorInitialClosestPoint;

	// Camera
	// We keep a series of quantities so that we can build a camera matrix without relying on the original
	// state of the camera. Instead, we build it from angles, etc.

	CrEditorCameraState m_cameraState;
};

extern CrEditor* Editor;