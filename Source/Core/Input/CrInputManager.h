#pragma once

#include "Core/Logging/ICrDebug.h"

#include "Core/Containers/CrBitSet.h"

#include "crstl/array.h"

namespace KeyboardKey
{
	enum Code
	{
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		Alpha0, Alpha1, Alpha2, Alpha3, Alpha4, Alpha5, Alpha6, Alpha7, Alpha8, Alpha9,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, 
		F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,
		Tab, CapsLock, LeftShift, RightShift, 
		Space, LeftCtrl, RightCtrl, Intro, Backspace, Alt, AltGr, Escape,
		UpArrow, DownArrow, LeftArrow, RightArrow, 
		Insert, PrintScreen, Delete, Home, End, PageUp, PageDown, Pause, Break, ScrollLock,
		NumLock, KeypadDivide, KeypadMultiply, KeypadMinus, KeypadPlus, KeypadEnter, KeypadPeriod,
		Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8, Keypad9, Keypad0,
		LeftWindowsCmd, RightWindowsCmd,

		Count
	};
};

namespace GamepadButton
{
	enum Code
	{
		A,
		B,
		X,
		Y,
		Back,
		Guide,
		Start,
		LeftStick,
		RightStick,
		LeftShoulder,
		RightShoulder,
		DpadUp,
		DpadDown,
		DpadLeft,
		DpadRight,
		Misc1,    // Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button
		Paddle1,  // Xbox Elite paddle P1
		Paddle2,  // Xbox Elite paddle P2
		Paddle3,  // Xbox Elite paddle P3
		Paddle4,  // Xbox Elite paddle P4
		Touchpad, // PS4/PS5 touchpad button
		L1,
		R1,
		Count
	};
};

namespace GamepadAxis
{
	enum Code
	{
		LeftX,
		LeftY,
		RightX,
		RightY,
		LeftTrigger,
		RightTrigger,
		Count
	};

	constexpr const char* ToString(GamepadAxis::Code code)
	{
		switch (code)
		{
			case LeftX: return "Left Axis X";
			case LeftY: return "Left Axis Y";
			case RightX: return "Right Axis X";
			case RightY: return "Right Axis Y";
			case LeftTrigger: return "Left Trigger";
			case RightTrigger: return "Right Trigger";
			default: return "";
		}
	}
};

namespace MouseButton
{
	enum Code
	{
		Left,
		Right,
		Middle,
		X1,
		X2,
		Count
	};
};

namespace crinput
{
	struct int2
	{
		int32_t x = 0;
		int32_t y = 0;
	};

	struct float2
	{
		float x = 0.0f;
		float y = 0.0f;
	};
}

struct MouseState
{
	// Whether mouse button was clicked (held and lifted)
	CrBitset<MouseButton::Count, uint8_t> buttonClicked;

	// Whether mouse button was pressed this tick (started to hold)
	CrBitset<MouseButton::Count, uint8_t> buttonPressed;

	// Whether mouse button is held down
	CrBitset<MouseButton::Count, uint8_t> buttonHeld;

	// Absolute position of mouse, in screen pixels
	crinput::int2 position;

	// Number of pixels the mouse has moved since the last time it was updated
	crinput::int2 relativePosition;

	crinput::float2 mouseWheel;
};

struct KeyboardState
{
	// Whether key was 'clicked' (held and raised)
	CrBitset<KeyboardKey::Count> keyClicked;

	// Whether key was pressed this tick (started to hold)
	CrBitset<KeyboardKey::Count> keyPressed;

	// Whether key is held down
	CrBitset<KeyboardKey::Count> keyHeld;
};

struct GamepadState
{
	bool connected = false;

	CrBitset<GamepadButton::Count, uint8_t> buttonPressed;

	crstl::array<float, GamepadAxis::Count> axes;
};

class CrInputManager
{
public:

	static const uint32_t MaxControllers = 8;

	static void Initialize();

	// Relative position values like mouse relative and wheel need to be reset on every update before we process
	// events, because these don't get 'reset' in any way
	void Update();

	const MouseState& GetMouseState() const;

	const KeyboardState& GetKeyboardState() const;

	const GamepadState& GetGamepadState(uint32_t index) const;

	void OnControllerConnected(uint32_t controllerIndex);

	void OnControllerDisconnected(uint32_t controllerIndex);

	void OnKeyboardDown(KeyboardKey::Code code);

	void OnKeyboardUp(KeyboardKey::Code code);

	void OnMouseMove(int32_t windowPositionX, int32_t windowPositionY, int32_t clientX, int32_t clientY);

	void OnMouseRelativeMove(int32_t relativeX, int32_t relativeY);

	void OnMouseWheelX(float x);

	void OnMouseWheelY(float y);

	void OnMouseButtonDown(MouseButton::Code code);

	void OnMouseButtonUp(MouseButton::Code code);

	void OnGamepadButton(uint32_t controllerIndex, GamepadButton::Code code, bool pressed);

	void OnGamepadAxis(uint32_t gamepadIndex, GamepadAxis::Code code, float value);

private:

	crstl::array<GamepadState, MaxControllers> m_gamepadStates;

	MouseState m_mouseState;

	KeyboardState m_keyboardState;
};

extern CrInputManager CrInput;