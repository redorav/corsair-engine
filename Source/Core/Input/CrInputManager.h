#pragma once

#include "Core/Logging/ICrDebug.h"

#include "Core/Containers/CrBitSet.h"

#include "Core/Containers/CrArray.h"

namespace KeyboardKey
{
	enum Code
	{
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		Alpha0, Alpha1, Alpha2, Alpha3, Alpha4, Alpha5, Alpha6, Alpha7, Alpha8, Alpha9,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, 
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

// Use SDL terminology as the abstraction
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

	crinput::int2 mouseWheel;
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

	CrArray<float, GamepadAxis::Count> axes;
};

class CrInputManager
{
public:

	static const uint32_t MaxControllers = 8;

	// Relative position values like mouse relative and wheel need to be reset on every update before we process
	// events, because these don't get 'reset' in any way
	void Update()
	{
		m_mouseState.relativePosition.x = 0;
		m_mouseState.relativePosition.y = 0;
		m_mouseState.mouseWheel.x = 0;
		m_mouseState.mouseWheel.y = 0;

		m_mouseState.buttonClicked = 0;
		m_mouseState.buttonPressed = 0;

		m_keyboardState.keyClicked = 0;
		m_keyboardState.keyPressed = 0;
	}

	const MouseState& GetMouseState() const
	{
		return m_mouseState;
	}

	const KeyboardState& GetKeyboardState() const
	{
		return m_keyboardState;
	}

	const GamepadState& GetGamepadState(uint32_t index) const
	{
		return m_gamepadStates[index];
	}

	void OnControllerConnected(uint32_t controllerIndex)
	{
		m_gamepadStates[controllerIndex].connected = true;
	}

	void OnControllerDisconnected(uint32_t controllerIndex)
	{
		m_gamepadStates[controllerIndex] = GamepadState();
	}

	void OnKeyboardDown(KeyboardKey::Code code)
	{
		m_keyboardState.keyPressed[code] = true;

		m_keyboardState.keyHeld[code] = true;
	}

	void OnKeyboardUp(KeyboardKey::Code code)
	{
		// If we used to be pressed but now we're not, we'll mark as clicked (for this frame)
		if (m_keyboardState.keyHeld[code])
		{
			m_keyboardState.keyClicked[code] = true;
		}

		m_keyboardState.keyHeld[code] = false;
	}

	void OnMouseMove(int32_t x, int32_t y)
	{
		m_mouseState.position.x = x;
		m_mouseState.position.y = y;
	}

	void OnMouseRelativeMove(int32_t x, int32_t y)
	{
		m_mouseState.relativePosition.x = x;
		m_mouseState.relativePosition.y = y;
	}

	void OnMouseWheel(int32_t x, int32_t y)
	{
		m_mouseState.mouseWheel.x = x;
		m_mouseState.mouseWheel.y = y;
	}

	void OnMouseButtonDown(MouseButton::Code code)
	{
		m_mouseState.buttonPressed[code] = true;

		m_mouseState.buttonHeld[code] = true;
	}

	void OnMouseButtonUp(MouseButton::Code code)
	{
		// If we used to be pressed but now we're not, we'll mark as clicked (for this frame)
		if (m_mouseState.buttonHeld[code])
		{
			m_mouseState.buttonClicked[code] = true;
		}

		m_mouseState.buttonHeld[code] = false;
	}

	void OnGamepadButtonDown(uint32_t controllerIndex, GamepadButton::Code code)
	{
		m_gamepadStates[controllerIndex].buttonPressed.set(code, true);
	}

	void OnGamepadButtonUp(uint32_t gamepadIndex, GamepadButton::Code code)
	{
		m_gamepadStates[gamepadIndex].buttonPressed.set(code, false);
	}

	void OnGamepadAxis(uint32_t gamepadIndex, GamepadAxis::Code code, float value)
	{
		float threshold = 0.3f;

		value = (value < threshold && value > -threshold) ? 0.0f : value;

		m_gamepadStates[gamepadIndex].axes[code] = value;
	}

private:

	CrArray<GamepadState, MaxControllers> m_gamepadStates;

	MouseState m_mouseState;

	KeyboardState m_keyboardState;
};

extern CrInputManager CrInput;