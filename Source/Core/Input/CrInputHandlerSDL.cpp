#include "Core/CrCore_pch.h"

#include "CrInputHandlerSDL.h"

CrInputHandlerSDL::CrInputHandlerSDL()
{
	SetupSDLInputMappings();
}

void CrInputHandlerSDL::HandleEvent(const SDL_Event& event)
{
	switch (event.type)
	{
		case SDL_EVENT_KEY_DOWN:
		{
			CrInput.OnKeyboardDown(m_keyboardMap[event.key.scancode]);
			break;
		}
		case SDL_EVENT_KEY_UP:
		{
			CrInput.OnKeyboardUp(m_keyboardMap[event.key.scancode]);
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		{
			CrInput.OnMouseButtonDown(m_mouseButtonMap[event.button.button]);
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			CrInput.OnMouseButtonUp(m_mouseButtonMap[event.button.button]);
			break;
		}
		case SDL_EVENT_MOUSE_MOTION:
		{
			CrInput.OnMouseMove((int32_t)event.motion.x, (int32_t)event.motion.y);
			CrInput.OnMouseRelativeMove((int32_t)event.motion.xrel, (int32_t)event.motion.yrel);
			break;
		}
		case SDL_EVENT_MOUSE_WHEEL:
		{
			CrInput.OnMouseWheel((int32_t)event.wheel.x, (int32_t)event.wheel.y);
			break;
		}
		case SDL_EVENT_GAMEPAD_ADDED:
		{
			m_connectedControllers[event.cdevice.which] = SDL_OpenGamepad(event.cdevice.which);
			CrInput.OnControllerConnected(event.cdevice.which);
			break;
		}
		case SDL_EVENT_GAMEPAD_REMOVED:
		{
			SDL_CloseGamepad(m_connectedControllers[event.cdevice.which]);
			CrInput.OnControllerDisconnected(event.cdevice.which);
			break;
		}
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		{
			CrInput.OnGamepadButtonDown(event.button.which, m_gamepadButtonMap[event.button.button]);
			break;
		}
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
		{
			CrInput.OnGamepadButtonUp(event.button.which, m_gamepadButtonMap[event.button.button]);
			break;
		}
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		{
			// Normalize the axis value (input range is -32768 to 32767)
			float axisNormalized = event.gaxis.value / (event.gaxis.value >= 0 ? 32767.0f : 32768.0f);

			GamepadAxis::Code axisCode = m_gamepadAxisMap[event.gaxis.axis];

			// Invert to keep it sensible
			if (axisCode == GamepadAxis::LeftY || axisCode == GamepadAxis::RightY)
			{
				axisNormalized *= -1.0f;
			}

			CrInput.OnGamepadAxis(event.gaxis.which, axisCode, axisNormalized);
			break;
		}
	}
}

void CrInputHandlerSDL::SetupSDLInputMappings()
{
	// Keyboard

	m_keyboardMap[SDL_SCANCODE_A] = KeyboardKey::A;
	m_keyboardMap[SDL_SCANCODE_B] = KeyboardKey::B;
	m_keyboardMap[SDL_SCANCODE_C] = KeyboardKey::C;
	m_keyboardMap[SDL_SCANCODE_D] = KeyboardKey::D;
	m_keyboardMap[SDL_SCANCODE_E] = KeyboardKey::E;
	m_keyboardMap[SDL_SCANCODE_F] = KeyboardKey::F;
	m_keyboardMap[SDL_SCANCODE_G] = KeyboardKey::G;
	m_keyboardMap[SDL_SCANCODE_H] = KeyboardKey::H;
	m_keyboardMap[SDL_SCANCODE_I] = KeyboardKey::I;
	m_keyboardMap[SDL_SCANCODE_J] = KeyboardKey::J;
	m_keyboardMap[SDL_SCANCODE_K] = KeyboardKey::K;
	m_keyboardMap[SDL_SCANCODE_L] = KeyboardKey::L;
	m_keyboardMap[SDL_SCANCODE_M] = KeyboardKey::M;
	m_keyboardMap[SDL_SCANCODE_N] = KeyboardKey::N;
	m_keyboardMap[SDL_SCANCODE_O] = KeyboardKey::O;
	m_keyboardMap[SDL_SCANCODE_P] = KeyboardKey::P;
	m_keyboardMap[SDL_SCANCODE_Q] = KeyboardKey::Q;
	m_keyboardMap[SDL_SCANCODE_R] = KeyboardKey::R;
	m_keyboardMap[SDL_SCANCODE_S] = KeyboardKey::S;
	m_keyboardMap[SDL_SCANCODE_T] = KeyboardKey::T;
	m_keyboardMap[SDL_SCANCODE_U] = KeyboardKey::U;
	m_keyboardMap[SDL_SCANCODE_V] = KeyboardKey::V;
	m_keyboardMap[SDL_SCANCODE_W] = KeyboardKey::W;
	m_keyboardMap[SDL_SCANCODE_X] = KeyboardKey::X;
	m_keyboardMap[SDL_SCANCODE_Y] = KeyboardKey::Y;
	m_keyboardMap[SDL_SCANCODE_Z] = KeyboardKey::Z;

	m_keyboardMap[SDL_SCANCODE_1] = KeyboardKey::Alpha1;
	m_keyboardMap[SDL_SCANCODE_2] = KeyboardKey::Alpha2;
	m_keyboardMap[SDL_SCANCODE_3] = KeyboardKey::Alpha3;
	m_keyboardMap[SDL_SCANCODE_4] = KeyboardKey::Alpha4;
	m_keyboardMap[SDL_SCANCODE_5] = KeyboardKey::Alpha5;
	m_keyboardMap[SDL_SCANCODE_6] = KeyboardKey::Alpha6;
	m_keyboardMap[SDL_SCANCODE_7] = KeyboardKey::Alpha7;
	m_keyboardMap[SDL_SCANCODE_8] = KeyboardKey::Alpha8;
	m_keyboardMap[SDL_SCANCODE_9] = KeyboardKey::Alpha9;
	m_keyboardMap[SDL_SCANCODE_0] = KeyboardKey::Alpha0;

	m_keyboardMap[SDL_SCANCODE_RETURN] = KeyboardKey::Intro;
	m_keyboardMap[SDL_SCANCODE_ESCAPE] = KeyboardKey::Escape;
	m_keyboardMap[SDL_SCANCODE_BACKSPACE] = KeyboardKey::Backspace;
	m_keyboardMap[SDL_SCANCODE_TAB] = KeyboardKey::Tab;
	m_keyboardMap[SDL_SCANCODE_SPACE] = KeyboardKey::Space;

	//m_keyboardMap[SDL_SCANCODE_MINUS = 45,
	//m_keyboardMap[SDL_SCANCODE_EQUALS = 46,
	//m_keyboardMap[SDL_SCANCODE_LEFTBRACKET = 47,
	//m_keyboardMap[SDL_SCANCODE_RIGHTBRACKET = 48,
	//m_keyboardMap[SDL_SCANCODE_BACKSLASH = 49,
	//m_keyboardMap[SDL_SCANCODE_NONUSHASH
	//m_keyboardMap[SDL_SCANCODE_SEMICOLON =
	//m_keyboardMap[SDL_SCANCODE_APOSTROPHE
	//m_keyboardMap[SDL_SCANCODE_GRAVE = 53,

	//m_keyboardMap[SDL_SCANCODE_COMMA] = 0;
	//m_keyboardMap[SDL_SCANCODE_PERIOD] = 0;
	//m_keyboardMap[SDL_SCANCODE_SLASH] = 0;

	m_keyboardMap[SDL_SCANCODE_CAPSLOCK] = KeyboardKey::CapsLock;

	m_keyboardMap[SDL_SCANCODE_F1] = KeyboardKey::F1;
	m_keyboardMap[SDL_SCANCODE_F2] = KeyboardKey::F2;
	m_keyboardMap[SDL_SCANCODE_F3] = KeyboardKey::F3;
	m_keyboardMap[SDL_SCANCODE_F4] = KeyboardKey::F4;
	m_keyboardMap[SDL_SCANCODE_F5] = KeyboardKey::F5;
	m_keyboardMap[SDL_SCANCODE_F6] = KeyboardKey::F6;
	m_keyboardMap[SDL_SCANCODE_F7] = KeyboardKey::F7;
	m_keyboardMap[SDL_SCANCODE_F8] = KeyboardKey::F8;
	m_keyboardMap[SDL_SCANCODE_F9] = KeyboardKey::F9;
	m_keyboardMap[SDL_SCANCODE_F10] = KeyboardKey::F10;
	m_keyboardMap[SDL_SCANCODE_F11] = KeyboardKey::F11;
	m_keyboardMap[SDL_SCANCODE_F12] = KeyboardKey::F12;

	m_keyboardMap[SDL_SCANCODE_PRINTSCREEN] = KeyboardKey::PrintScreen;
	m_keyboardMap[SDL_SCANCODE_SCROLLLOCK]  = KeyboardKey::ScrollLock;
	m_keyboardMap[SDL_SCANCODE_PAUSE]       = KeyboardKey::Pause;
	m_keyboardMap[SDL_SCANCODE_INSERT]      = KeyboardKey::Insert;
	m_keyboardMap[SDL_SCANCODE_HOME]        = KeyboardKey::Home;
	m_keyboardMap[SDL_SCANCODE_PAGEUP]      = KeyboardKey::PageUp;
	m_keyboardMap[SDL_SCANCODE_DELETE]      = KeyboardKey::Delete;
	m_keyboardMap[SDL_SCANCODE_END]         = KeyboardKey::End;
	m_keyboardMap[SDL_SCANCODE_PAGEDOWN]    = KeyboardKey::PageDown;

	m_keyboardMap[SDL_SCANCODE_RIGHT] = KeyboardKey::RightArrow;
	m_keyboardMap[SDL_SCANCODE_LEFT]  = KeyboardKey::LeftArrow;
	m_keyboardMap[SDL_SCANCODE_DOWN]  = KeyboardKey::DownArrow;
	m_keyboardMap[SDL_SCANCODE_UP]    = KeyboardKey::UpArrow;

	m_keyboardMap[SDL_SCANCODE_NUMLOCKCLEAR] = KeyboardKey::NumLock;

	m_keyboardMap[SDL_SCANCODE_KP_DIVIDE]   = KeyboardKey::KeypadDivide;
	m_keyboardMap[SDL_SCANCODE_KP_MULTIPLY] = KeyboardKey::KeypadMultiply;
	m_keyboardMap[SDL_SCANCODE_KP_MINUS]    = KeyboardKey::KeypadMinus;
	m_keyboardMap[SDL_SCANCODE_KP_PLUS]     = KeyboardKey::KeypadPlus;
	m_keyboardMap[SDL_SCANCODE_KP_ENTER]    = KeyboardKey::KeypadEnter;
	m_keyboardMap[SDL_SCANCODE_KP_1]        = KeyboardKey::Keypad1;
	m_keyboardMap[SDL_SCANCODE_KP_2]        = KeyboardKey::Keypad2;
	m_keyboardMap[SDL_SCANCODE_KP_3]        = KeyboardKey::Keypad3;
	m_keyboardMap[SDL_SCANCODE_KP_4]        = KeyboardKey::Keypad4;
	m_keyboardMap[SDL_SCANCODE_KP_5]        = KeyboardKey::Keypad5;
	m_keyboardMap[SDL_SCANCODE_KP_6]        = KeyboardKey::Keypad6;
	m_keyboardMap[SDL_SCANCODE_KP_7]        = KeyboardKey::Keypad7;
	m_keyboardMap[SDL_SCANCODE_KP_8]        = KeyboardKey::Keypad8;
	m_keyboardMap[SDL_SCANCODE_KP_9]        = KeyboardKey::Keypad9;
	m_keyboardMap[SDL_SCANCODE_KP_0]        = KeyboardKey::Keypad0;
	m_keyboardMap[SDL_SCANCODE_KP_PERIOD]   = KeyboardKey::KeypadPeriod;

	m_keyboardMap[SDL_SCANCODE_LCTRL]  = KeyboardKey::LeftCtrl;
	m_keyboardMap[SDL_SCANCODE_LSHIFT] = KeyboardKey::LeftShift;
	m_keyboardMap[SDL_SCANCODE_LALT]   = KeyboardKey::Alt; // Alt, Option
	m_keyboardMap[SDL_SCANCODE_LGUI]   = KeyboardKey::LeftWindowsCmd; // Windows, Command (Apple), meta
	m_keyboardMap[SDL_SCANCODE_RCTRL]  = KeyboardKey::RightCtrl;
	m_keyboardMap[SDL_SCANCODE_RSHIFT] = KeyboardKey::RightShift;
	m_keyboardMap[SDL_SCANCODE_RALT]   = KeyboardKey::AltGr; // Alt Gr, Option
	m_keyboardMap[SDL_SCANCODE_RGUI]   = KeyboardKey::RightWindowsCmd; // Windows, Command (Apple), meta

	// Mouse

	m_mouseButtonMap[SDL_BUTTON_LEFT]   = MouseButton::Left;
	m_mouseButtonMap[SDL_BUTTON_RIGHT]  = MouseButton::Right;
	m_mouseButtonMap[SDL_BUTTON_MIDDLE] = MouseButton::Middle;
	m_mouseButtonMap[SDL_BUTTON_X1]     = MouseButton::X1;
	m_mouseButtonMap[SDL_BUTTON_X2]     = MouseButton::X2;

	// Gamepad

	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_SOUTH]          = GamepadButton::A;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_EAST]           = GamepadButton::B;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_WEST]           = GamepadButton::X;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_NORTH]          = GamepadButton::Y;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_BACK]           = GamepadButton::Back;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_GUIDE]          = GamepadButton::Guide;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_START]          = GamepadButton::Start;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_LEFT_STICK]     = GamepadButton::LeftStick;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_RIGHT_STICK]    = GamepadButton::RightStick;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_LEFT_SHOULDER]  = GamepadButton::LeftShoulder;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER] = GamepadButton::RightShoulder;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_DPAD_UP]        = GamepadButton::DpadUp;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_DPAD_DOWN]      = GamepadButton::DpadDown;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_DPAD_LEFT]      = GamepadButton::DpadLeft;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_DPAD_RIGHT]     = GamepadButton::DpadRight;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_MISC1]          = GamepadButton::Misc1;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_LEFT_PADDLE1]   = GamepadButton::Paddle1;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_LEFT_PADDLE2]   = GamepadButton::Paddle2;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1]  = GamepadButton::Paddle3;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2]  = GamepadButton::Paddle4;
	m_gamepadButtonMap[SDL_GAMEPAD_BUTTON_TOUCHPAD]       = GamepadButton::Touchpad;

	m_gamepadAxisMap[SDL_GAMEPAD_AXIS_LEFTX]         = GamepadAxis::LeftX;
	m_gamepadAxisMap[SDL_GAMEPAD_AXIS_LEFTY]         = GamepadAxis::LeftY;
	m_gamepadAxisMap[SDL_GAMEPAD_AXIS_RIGHTX]        = GamepadAxis::RightX;
	m_gamepadAxisMap[SDL_GAMEPAD_AXIS_RIGHTY]        = GamepadAxis::RightY;
	m_gamepadAxisMap[SDL_GAMEPAD_AXIS_LEFT_TRIGGER]  = GamepadAxis::LeftTrigger;
	m_gamepadAxisMap[SDL_GAMEPAD_AXIS_RIGHT_TRIGGER] = GamepadAxis::RightTrigger;
}
