#pragma once

#include <SDL3/SDL.h>

#include "CrInputManager.h"

extern CrInputManager CrInput;

class CrInputHandlerSDL
{
public:

	CrInputHandlerSDL();

	void HandleEvent(const SDL_Event& event);

private:

	void SetupSDLInputMappings();

	CrArray<SDL_Gamepad*, CrInputManager::MaxControllers> m_connectedControllers = {};

	CrArray<GamepadButton::Code, SDL_GAMEPAD_BUTTON_COUNT> m_gamepadButtonMap;

	CrArray<GamepadAxis::Code, SDL_GAMEPAD_AXIS_COUNT> m_gamepadAxisMap;

	CrArray<KeyboardKey::Code, SDL_SCANCODE_COUNT> m_keyboardMap;

	CrArray<MouseButton::Code, SDL_BUTTON_X2 + 1> m_mouseButtonMap;
};