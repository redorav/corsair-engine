#pragma once

#pragma warning(push, 0) // Suppress external warnings
#include <gainput/gainput.h> // Platform independent input library (valid for PC, Mac, Linux, Android and iOS)
#pragma warning(pop)

enum KeyCode
{
	A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	Alpha0, Alpha1, Alpha2, Alpha3, Alpha4, Alpha5, Alpha6, Alpha7, Alpha8, Alpha9,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, Space, LeftCtrl, RightCtrl,
	Intro, Backspace, LeftShift, RightShift, Alt, AltGr, 
	UpArrow, DownArrow, LeftArrow, RightArrow, Tab,

	MouseLeft,
	MouseRight,
	MouseMiddle,

	JoystickButtonA,
	JoystickButtonB,
	JoystickButtonX,
	JoystickButtonY,	
};

enum AxisCode
{
	JoystickLeftAxisX,
	JoystickLeftAxisY,
	JoystickRightAxisX,
	JoystickRightAxisY,

	JoystickL1,
	JoystickL2,
	JoystickR1,
	JoystickR2,

	MouseX,
	MouseY
};

class CrInputManager
{
public:

	CrInputManager()
	{
		m_keyboardId = m_inputManager.CreateDevice<gainput::InputDeviceKeyboard>();
		m_mouseId = m_inputManager.CreateDevice<gainput::InputDeviceMouse>();
		m_padId = m_inputManager.CreateDevice<gainput::InputDevicePad>();

		// TODO: Add ability to have more pads

		m_inputManager.SetDisplaySize(1280, 720); // What is this needed for?
		m_keyInputMap = new gainput::InputMap(m_inputManager);
		m_axisInputMap = new gainput::InputMap(m_inputManager);

		SetupGainputMappings();
	}

	~CrInputManager()
	{
		delete m_keyInputMap;
		delete m_axisInputMap;
	}

	bool GetKey(KeyCode key)
	{
		return m_keyInputMap->GetBoolWasDown((gainput::UserButtonId)key);
	}

	bool GetHeldKey(KeyCode key)
	{
		return m_keyInputMap->GetBool((gainput::UserButtonId)key);
	}

	float GetAxis(AxisCode axis)
	{
		return m_axisInputMap->GetFloat((gainput::UserButtonId)axis);
	}

	void Update()
	{
		m_inputManager.Update();
	}

	void HandleMessage(void* message)
	{
#if defined(_WIN32) // TODO use define coming from Premake
		MSG& msg = *static_cast<MSG*>(message);
		m_inputManager.HandleMessage(msg);
#endif
	}

private:

	void SetupGainputMappings()
	{
		assert(m_keyInputMap != nullptr);

		m_keyInputMap->MapBool(KeyCode::A, m_keyboardId, gainput::KeyA);
		m_keyInputMap->MapBool(KeyCode::B, m_keyboardId, gainput::KeyB);
		m_keyInputMap->MapBool(KeyCode::C, m_keyboardId, gainput::KeyC);
		m_keyInputMap->MapBool(KeyCode::D, m_keyboardId, gainput::KeyD);
		m_keyInputMap->MapBool(KeyCode::E, m_keyboardId, gainput::KeyE);
		m_keyInputMap->MapBool(KeyCode::F, m_keyboardId, gainput::KeyF);
		m_keyInputMap->MapBool(KeyCode::G, m_keyboardId, gainput::KeyG);
		m_keyInputMap->MapBool(KeyCode::H, m_keyboardId, gainput::KeyH);
		m_keyInputMap->MapBool(KeyCode::I, m_keyboardId, gainput::KeyI);
		m_keyInputMap->MapBool(KeyCode::J, m_keyboardId, gainput::KeyJ);
		m_keyInputMap->MapBool(KeyCode::K, m_keyboardId, gainput::KeyK);
		m_keyInputMap->MapBool(KeyCode::L, m_keyboardId, gainput::KeyL);
		m_keyInputMap->MapBool(KeyCode::M, m_keyboardId, gainput::KeyM);
		m_keyInputMap->MapBool(KeyCode::N, m_keyboardId, gainput::KeyN);
		m_keyInputMap->MapBool(KeyCode::O, m_keyboardId, gainput::KeyO);
		m_keyInputMap->MapBool(KeyCode::P, m_keyboardId, gainput::KeyP);
		m_keyInputMap->MapBool(KeyCode::Q, m_keyboardId, gainput::KeyQ);
		m_keyInputMap->MapBool(KeyCode::R, m_keyboardId, gainput::KeyR);
		m_keyInputMap->MapBool(KeyCode::S, m_keyboardId, gainput::KeyS);
		m_keyInputMap->MapBool(KeyCode::T, m_keyboardId, gainput::KeyT);
		m_keyInputMap->MapBool(KeyCode::U, m_keyboardId, gainput::KeyU);
		m_keyInputMap->MapBool(KeyCode::V, m_keyboardId, gainput::KeyV);
		m_keyInputMap->MapBool(KeyCode::W, m_keyboardId, gainput::KeyW);
		m_keyInputMap->MapBool(KeyCode::X, m_keyboardId, gainput::KeyX);
		m_keyInputMap->MapBool(KeyCode::Y, m_keyboardId, gainput::KeyY);
		m_keyInputMap->MapBool(KeyCode::Z, m_keyboardId, gainput::KeyZ);

		m_keyInputMap->MapBool(KeyCode::Alpha0, m_keyboardId, gainput::Key0);
		m_keyInputMap->MapBool(KeyCode::Alpha1, m_keyboardId, gainput::Key1);
		m_keyInputMap->MapBool(KeyCode::Alpha2, m_keyboardId, gainput::Key2);
		m_keyInputMap->MapBool(KeyCode::Alpha3, m_keyboardId, gainput::Key3);
		m_keyInputMap->MapBool(KeyCode::Alpha4, m_keyboardId, gainput::Key4);
		m_keyInputMap->MapBool(KeyCode::Alpha5, m_keyboardId, gainput::Key5);
		m_keyInputMap->MapBool(KeyCode::Alpha6, m_keyboardId, gainput::Key6);
		m_keyInputMap->MapBool(KeyCode::Alpha7, m_keyboardId, gainput::Key7);
		m_keyInputMap->MapBool(KeyCode::Alpha8, m_keyboardId, gainput::Key8);
		m_keyInputMap->MapBool(KeyCode::Alpha9, m_keyboardId, gainput::Key9);
		m_keyInputMap->MapBool(KeyCode::F1, m_keyboardId, gainput::KeyF1);
		m_keyInputMap->MapBool(KeyCode::F2, m_keyboardId, gainput::KeyF2);
		m_keyInputMap->MapBool(KeyCode::F3, m_keyboardId, gainput::KeyF3);
		m_keyInputMap->MapBool(KeyCode::F4, m_keyboardId, gainput::KeyF4);
		m_keyInputMap->MapBool(KeyCode::F5, m_keyboardId, gainput::KeyF5);
		m_keyInputMap->MapBool(KeyCode::F6, m_keyboardId, gainput::KeyF6);
		m_keyInputMap->MapBool(KeyCode::F7, m_keyboardId, gainput::KeyF7);
		m_keyInputMap->MapBool(KeyCode::F8, m_keyboardId, gainput::KeyF8);
		m_keyInputMap->MapBool(KeyCode::F9, m_keyboardId, gainput::KeyF9);
		m_keyInputMap->MapBool(KeyCode::F10, m_keyboardId, gainput::KeyF10);
		m_keyInputMap->MapBool(KeyCode::F11, m_keyboardId, gainput::KeyF11);
		m_keyInputMap->MapBool(KeyCode::F12, m_keyboardId, gainput::KeyF12);
		m_keyInputMap->MapBool(KeyCode::Space,		m_keyboardId, gainput::KeySpace);
		m_keyInputMap->MapBool(KeyCode::LeftCtrl,	m_keyboardId, gainput::KeyCtrlL);
		m_keyInputMap->MapBool(KeyCode::RightCtrl, m_keyboardId, gainput::KeyCtrlR);
		m_keyInputMap->MapBool(KeyCode::Intro,		m_keyboardId, gainput::KeyKpEnter);

		m_keyInputMap->MapBool(KeyCode::Backspace,		m_keyboardId, gainput::KeyF7);
		m_keyInputMap->MapBool(KeyCode::LeftShift,		m_keyboardId, gainput::KeyF8);
		m_keyInputMap->MapBool(KeyCode::RightShift,	m_keyboardId, gainput::KeyF9);
		m_keyInputMap->MapBool(KeyCode::Alt,			m_keyboardId, gainput::KeyF10);
		m_keyInputMap->MapBool(KeyCode::AltGr,			m_keyboardId, gainput::KeyF11);
		m_keyInputMap->MapBool(KeyCode::UpArrow,		m_keyboardId, gainput::KeyF12);
		m_keyInputMap->MapBool(KeyCode::DownArrow,		m_keyboardId, gainput::KeySpace);
		m_keyInputMap->MapBool(KeyCode::LeftArrow,		m_keyboardId, gainput::KeyCtrlL);
		m_keyInputMap->MapBool(KeyCode::RightArrow,	m_keyboardId, gainput::KeyCtrlR);
		m_keyInputMap->MapBool(KeyCode::Tab,			m_keyboardId, gainput::KeyKpEnter);

		m_keyInputMap->MapBool(KeyCode::MouseLeft, m_mouseId, gainput::MouseButton0);
		m_keyInputMap->MapBool(KeyCode::MouseRight, m_mouseId, gainput::MouseButton1);
		m_keyInputMap->MapBool(KeyCode::MouseMiddle, m_mouseId, gainput::MouseButton2);

		m_keyInputMap->MapBool(KeyCode::JoystickButtonA, m_padId, gainput::PadButtonA);
		m_keyInputMap->MapBool(KeyCode::JoystickButtonB, m_padId, gainput::PadButtonB);
		m_keyInputMap->MapBool(KeyCode::JoystickButtonX, m_padId, gainput::PadButtonX);
		m_keyInputMap->MapBool(KeyCode::JoystickButtonY, m_padId, gainput::PadButtonY);


		m_axisInputMap->MapFloat(AxisCode::JoystickLeftAxisX, m_padId, gainput::PadButtonLeftStickX);
		m_axisInputMap->MapFloat(AxisCode::JoystickLeftAxisY, m_padId, gainput::PadButtonLeftStickY);
		m_axisInputMap->MapFloat(AxisCode::JoystickRightAxisX, m_padId, gainput::PadButtonRightStickX);
		m_axisInputMap->MapFloat(AxisCode::JoystickRightAxisY, m_padId, gainput::PadButtonRightStickY);

		m_axisInputMap->MapFloat(AxisCode::JoystickL1, m_padId, gainput::PadButtonL1);
		m_axisInputMap->MapFloat(AxisCode::JoystickL2, m_padId, gainput::PadButtonL2);
		m_axisInputMap->MapFloat(AxisCode::JoystickR1, m_padId, gainput::PadButtonR1);
		m_axisInputMap->MapFloat(AxisCode::JoystickR2, m_padId, gainput::PadButtonR2);

		m_axisInputMap->MapFloat(AxisCode::MouseX, m_mouseId, gainput::MouseAxisX);
		m_axisInputMap->MapFloat(AxisCode::MouseY, m_mouseId, gainput::MouseAxisY);
	}

	gainput::InputManager m_inputManager;

	gainput::DeviceId m_keyboardId;
	gainput::DeviceId m_mouseId;
	gainput::DeviceId m_padId;

	gainput::InputMap* m_keyInputMap;
	gainput::InputMap* m_axisInputMap;
};

extern CrInputManager CrInput;