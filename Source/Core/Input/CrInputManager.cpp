#include "Core/CrCore_pch.h"

#include "CrInputManager.h"

#include "CrPlatformInput.h"

#include <imgui.h>

CrInputManager CrInput;

static ImGuiKey KeyboardKeyToImguiKey[KeyboardKey::Code::Count] = {};

void CrInputManager::Initialize()
{
	KeyboardKeyToImguiKey[KeyboardKey::Tab]         = ImGuiKey_Tab;
	KeyboardKeyToImguiKey[KeyboardKey::CapsLock]    = ImGuiKey_CapsLock;
	
	KeyboardKeyToImguiKey[KeyboardKey::LeftArrow]   = ImGuiKey_LeftArrow;
	KeyboardKeyToImguiKey[KeyboardKey::RightArrow]  = ImGuiKey_RightArrow;
	KeyboardKeyToImguiKey[KeyboardKey::UpArrow]     = ImGuiKey_UpArrow;
	KeyboardKeyToImguiKey[KeyboardKey::DownArrow]   = ImGuiKey_DownArrow;

	KeyboardKeyToImguiKey[KeyboardKey::PageUp]      = ImGuiKey_PageUp;
	KeyboardKeyToImguiKey[KeyboardKey::PageDown]    = ImGuiKey_PageDown;
	KeyboardKeyToImguiKey[KeyboardKey::Home]        = ImGuiKey_Home;
	KeyboardKeyToImguiKey[KeyboardKey::End]         = ImGuiKey_End;
	KeyboardKeyToImguiKey[KeyboardKey::Insert]      = ImGuiKey_Insert;
	KeyboardKeyToImguiKey[KeyboardKey::Delete]      = ImGuiKey_Delete;
	KeyboardKeyToImguiKey[KeyboardKey::Backspace]   = ImGuiKey_Backspace;
	KeyboardKeyToImguiKey[KeyboardKey::Space]       = ImGuiKey_Space;
	KeyboardKeyToImguiKey[KeyboardKey::Intro]       = ImGuiKey_Enter;
	KeyboardKeyToImguiKey[KeyboardKey::Escape]      = ImGuiKey_Escape;
	KeyboardKeyToImguiKey[KeyboardKey::KeypadEnter] = ImGuiKey_KeypadEnter;

	KeyboardKeyToImguiKey[KeyboardKey::Alt]         = ImGuiKey_LeftAlt;
	KeyboardKeyToImguiKey[KeyboardKey::AltGr]       = ImGuiKey_RightAlt;

	KeyboardKeyToImguiKey[KeyboardKey::LeftShift]   = ImGuiKey_LeftShift;
	KeyboardKeyToImguiKey[KeyboardKey::RightShift]  = ImGuiKey_RightShift;

	KeyboardKeyToImguiKey[KeyboardKey::LeftCtrl]    = ImGuiKey_LeftCtrl;
	KeyboardKeyToImguiKey[KeyboardKey::RightCtrl]   = ImGuiKey_RightCtrl;

	KeyboardKeyToImguiKey[KeyboardKey::A]           = ImGuiKey_A;
	KeyboardKeyToImguiKey[KeyboardKey::B]           = ImGuiKey_B;
	KeyboardKeyToImguiKey[KeyboardKey::C]           = ImGuiKey_C;
	KeyboardKeyToImguiKey[KeyboardKey::D]           = ImGuiKey_D;
	KeyboardKeyToImguiKey[KeyboardKey::E]           = ImGuiKey_E;
	KeyboardKeyToImguiKey[KeyboardKey::F]           = ImGuiKey_F;
	KeyboardKeyToImguiKey[KeyboardKey::G]           = ImGuiKey_G;
	KeyboardKeyToImguiKey[KeyboardKey::H]           = ImGuiKey_H;
	KeyboardKeyToImguiKey[KeyboardKey::I]           = ImGuiKey_I;
	KeyboardKeyToImguiKey[KeyboardKey::J]           = ImGuiKey_J;
	KeyboardKeyToImguiKey[KeyboardKey::K]           = ImGuiKey_K;
	KeyboardKeyToImguiKey[KeyboardKey::L]           = ImGuiKey_L;
	KeyboardKeyToImguiKey[KeyboardKey::M]           = ImGuiKey_M;
	KeyboardKeyToImguiKey[KeyboardKey::N]           = ImGuiKey_N;
	KeyboardKeyToImguiKey[KeyboardKey::O]           = ImGuiKey_O;
	KeyboardKeyToImguiKey[KeyboardKey::P]           = ImGuiKey_P;
	KeyboardKeyToImguiKey[KeyboardKey::Q]           = ImGuiKey_Q;
	KeyboardKeyToImguiKey[KeyboardKey::R]           = ImGuiKey_R;
	KeyboardKeyToImguiKey[KeyboardKey::S]           = ImGuiKey_S;
	KeyboardKeyToImguiKey[KeyboardKey::T]           = ImGuiKey_T;
	KeyboardKeyToImguiKey[KeyboardKey::U]           = ImGuiKey_U;
	KeyboardKeyToImguiKey[KeyboardKey::V]           = ImGuiKey_V;
	KeyboardKeyToImguiKey[KeyboardKey::W]           = ImGuiKey_W;
	KeyboardKeyToImguiKey[KeyboardKey::X]           = ImGuiKey_X;
	KeyboardKeyToImguiKey[KeyboardKey::Y]           = ImGuiKey_Y;
	KeyboardKeyToImguiKey[KeyboardKey::Z]           = ImGuiKey_Z;

	CrPlatformInput::Initialize();
}

void CrInputManager::Update()
{
	// Reset current input

	m_mouseState.relativePosition.x = 0;
	m_mouseState.relativePosition.y = 0;
	m_mouseState.mouseWheel.x = 0;
	m_mouseState.mouseWheel.y = 0;

	m_mouseState.buttonClicked = 0;
	m_mouseState.buttonPressed = 0;

	m_keyboardState.keyClicked = 0;
	m_keyboardState.keyPressed = 0;

	// Update platform input, which populates input structures accordingly

	PlatformInput->Update();
}

const MouseState& CrInputManager::GetMouseState() const
{
	return m_mouseState;
}

const KeyboardState& CrInputManager::GetKeyboardState() const
{
	return m_keyboardState;
}

const GamepadState& CrInputManager::GetGamepadState(uint32_t index) const
{
	return m_gamepadStates[index];
}

void CrInputManager::OnControllerConnected(uint32_t controllerIndex)
{
	m_gamepadStates[controllerIndex].connected = true;
}

void CrInputManager::OnControllerDisconnected(uint32_t controllerIndex)
{
	m_gamepadStates[controllerIndex] = GamepadState();
}

void CrInputManager::OnKeyboardDown(KeyboardKey::Code code)
{
	m_keyboardState.keyPressed[code] = true;

	m_keyboardState.keyHeld[code] = true;

	ImGuiIO& io = ImGui::GetIO();
	io.AddKeyEvent(KeyboardKeyToImguiKey[code], true);
}

void CrInputManager::OnKeyboardUp(KeyboardKey::Code code)
{
	// If we used to be pressed but now we're not, we'll mark as clicked (for this frame)
	if (m_keyboardState.keyHeld[code])
	{
		m_keyboardState.keyClicked[code] = true;
	}

	m_keyboardState.keyHeld[code] = false;

	ImGuiIO& io = ImGui::GetIO();
	io.AddKeyEvent(KeyboardKeyToImguiKey[code], false);
}

void CrInputManager::OnMouseMove(int32_t absoluteX, int32_t absoluteY, int32_t clientX, int32_t clientY)
{
	m_mouseState.position.x = clientX;
	m_mouseState.position.y = clientY;

	ImGuiIO& io = ImGui::GetIO();
	bool wantAbsolutePosition = (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0;
	
	float mousePosX = (float)clientX;
	float mousePosY = (float)clientY;

	if (wantAbsolutePosition)
	{
		mousePosX = (float)absoluteX;
		mousePosY = (float)absoluteY;
	}

	io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
	io.AddMousePosEvent(mousePosX, mousePosY);
}

void CrInputManager::OnMouseRelativeMove(int32_t relativeX, int32_t relativeY)
{
	m_mouseState.relativePosition.x = relativeX;
	m_mouseState.relativePosition.y = relativeY;
}

void CrInputManager::OnMouseWheelX(float x)
{
	m_mouseState.mouseWheel.x = x;

	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
	io.AddMouseWheelEvent(x, 0.0f);
}

void CrInputManager::OnMouseWheelY(float y)
{
	m_mouseState.mouseWheel.y = y;

	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
	io.AddMouseWheelEvent(0.0f, y);
}

void CrInputManager::OnMouseButtonDown(MouseButton::Code code)
{
	m_mouseState.buttonPressed[code] = true;

	m_mouseState.buttonHeld[code] = true;

	ImGuiIO& io = ImGui::GetIO();
	int imguiMouseButton = 0;
	switch (code)
	{
		case MouseButton::Left: imguiMouseButton = 0; break;
		case MouseButton::Right: imguiMouseButton = 1; break;
		case MouseButton::Middle: imguiMouseButton = 2; break;
		default: break;
	}

	io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
	io.AddMouseButtonEvent(imguiMouseButton, true);
}

void CrInputManager::OnMouseButtonUp(MouseButton::Code code)
{
	// If we used to be pressed but now we're not, we'll mark as clicked (for this frame)
	if (m_mouseState.buttonHeld[code])
	{
		m_mouseState.buttonClicked[code] = true;
	}

	m_mouseState.buttonHeld[code] = false;

	ImGuiIO& io = ImGui::GetIO();

	int imguiMouseButton = 0;
	switch (code)
	{
		case MouseButton::Left: imguiMouseButton = 0; break;
		case MouseButton::Right: imguiMouseButton = 1; break;
		case MouseButton::Middle: imguiMouseButton = 2; break;
		default: break;
	}

	io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
	io.AddMouseButtonEvent(imguiMouseButton, false);
}

void CrInputManager::OnGamepadButton(uint32_t controllerIndex, GamepadButton::Code code, bool pressed)
{
	m_gamepadStates[controllerIndex].buttonPressed.set(code, pressed);
}

void CrInputManager::OnGamepadAxis(uint32_t gamepadIndex, GamepadAxis::Code code, float value)
{
	float threshold = 0.3f;

	value = (value < threshold && value > -threshold) ? 0.0f : value;

	m_gamepadStates[gamepadIndex].axes[code] = value;
}
