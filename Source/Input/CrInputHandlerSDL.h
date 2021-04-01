#pragma once

#include <SDL.h>

#include "CrInputManager.h"

extern CrInputManager CrInput;

class CrInputHandlerSDL
{
public:

	CrInputHandlerSDL();

	void HandleEvent(const SDL_Event& event);

private:

	void SetupSDLInputMappings();

	CrArray<SDL_GameController*, CrInputManager::MaxControllers> m_connectedControllers = {};

	CrArray<GamepadButton::Code, SDL_CONTROLLER_BUTTON_MAX> m_controllerButtonMap;

	CrArray<GamepadAxis::Code, SDL_CONTROLLER_AXIS_MAX> m_controllerAxisMap;

	CrArray<KeyboardKey::Code, SDL_NUM_SCANCODES> m_keyboardMap;

	CrArray<MouseButton::Code, SDL_BUTTON_X2 + 1> m_mouseButtonMap;
};