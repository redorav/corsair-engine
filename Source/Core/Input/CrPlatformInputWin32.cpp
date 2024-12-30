#include "Core/CrCore_pch.h"

#include "Core/Input/CrInputManager.h"
#include "Core/Input/CrPlatformInput.h"

#include <windows.h>
#include <hidusage.h>

#include <Xinput.h>

CrPlatformInput* PlatformInput;

static XINPUT_STATE CurrentXInputState;

void CrPlatformInput::Initialize()
{
	PlatformInput = new CrPlatformInput();
}

void CrPlatformInput::Deinitialize()
{
	delete PlatformInput;
	PlatformInput = nullptr;
}

void CrPlatformInput::RequestRefreshControllers()
{
	if (!m_requestControllersRefresh)
	{
		for (uint32_t i = 0; i < MaxControllers; ++i)
		{
			m_previousControllerStatus[i] = m_controllerStatus[i];
			m_controllerStatus[i] = ConnectionStatus::Unknown;
		}
	}

	m_requestControllersRefresh = true;
}

void CrPlatformInput::Update()
{
	for (uint32_t i = 0; i < MaxControllers; ++i)
	{
		if (m_controllerStatus[i] == ConnectionStatus::Unknown)
		{
			XINPUT_CAPABILITIES xInputCapabilities = {};
			if (XInputGetCapabilities(i, XINPUT_FLAG_GAMEPAD, &xInputCapabilities) == ERROR_SUCCESS)
			{
				if (m_previousControllerStatus[i] != ConnectionStatus::Connected)
				{
					CrInput.OnControllerConnected(i);
				}

				m_controllerStatus[i] = ConnectionStatus::Connected;
			}
			else
			{
				if (m_previousControllerStatus[i] != ConnectionStatus::Disconnected)
				{
					CrInput.OnControllerDisconnected(i);
				}

				m_controllerStatus[i] = ConnectionStatus::Disconnected;
			}
		}

		if (m_controllerStatus[i] == ConnectionStatus::Connected)
		{
			XINPUT_STATE state = {};

			DWORD dwResult = XInputGetState(i, &state);

			if (CurrentXInputState.dwPacketNumber != state.dwPacketNumber)
			{
				if (dwResult == ERROR_SUCCESS)
				{
					CrInput.OnGamepadButton(i, GamepadButton::DpadUp, state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
					CrInput.OnGamepadButton(i, GamepadButton::DpadDown, state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
					CrInput.OnGamepadButton(i, GamepadButton::DpadLeft, state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
					CrInput.OnGamepadButton(i, GamepadButton::DpadRight, state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

					CrInput.OnGamepadButton(i, GamepadButton::Start, state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
					CrInput.OnGamepadButton(i, GamepadButton::Back, state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);

					CrInput.OnGamepadButton(i, GamepadButton::LeftStick, state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
					CrInput.OnGamepadButton(i, GamepadButton::RightStick, state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);

					CrInput.OnGamepadButton(i, GamepadButton::LeftShoulder, state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
					CrInput.OnGamepadButton(i, GamepadButton::RightShoulder, state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

					CrInput.OnGamepadButton(i, GamepadButton::A, state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
					CrInput.OnGamepadButton(i, GamepadButton::B, state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
					CrInput.OnGamepadButton(i, GamepadButton::X, state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
					CrInput.OnGamepadButton(i, GamepadButton::Y, state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);

					float leftTrigger = state.Gamepad.bLeftTrigger / 256.0f;
					float rightTrigger = state.Gamepad.bRightTrigger / 256.0f;

					float leftAxisX = state.Gamepad.sThumbLX > 0.0f ? state.Gamepad.sThumbLX / 32767.0f : state.Gamepad.sThumbLX / 32768.0f;
					float leftAxisY = state.Gamepad.sThumbLY > 0.0f ? state.Gamepad.sThumbLY / 32767.0f : state.Gamepad.sThumbLY / 32768.0f;

					float rightAxisX = state.Gamepad.sThumbRX > 0.0f ? state.Gamepad.sThumbRX / 32767.0f : state.Gamepad.sThumbRX / 32768.0f;
					float rightAxisY = state.Gamepad.sThumbRY > 0.0f ? state.Gamepad.sThumbRY / 32767.0f : state.Gamepad.sThumbRY / 32768.0f;

					CrInput.OnGamepadAxis(i, GamepadAxis::LeftTrigger, leftTrigger);
					CrInput.OnGamepadAxis(i, GamepadAxis::RightTrigger, rightTrigger);

					CrInput.OnGamepadAxis(i, GamepadAxis::LeftX, leftAxisX);
					CrInput.OnGamepadAxis(i, GamepadAxis::LeftY, leftAxisY);

					CrInput.OnGamepadAxis(i, GamepadAxis::RightX, rightAxisX);
					CrInput.OnGamepadAxis(i, GamepadAxis::RightY, rightAxisY);
				}

				CurrentXInputState = state;
			}
		}
	}

	m_requestControllersRefresh = false;
}