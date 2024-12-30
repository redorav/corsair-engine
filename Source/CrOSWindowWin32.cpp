#include "CrOSWindow.h"

#include "Core/Logging/ICrDebug.h"
#include "Core/Input/CrInputManager.h"
#include "Core/Input/CrPlatformInput.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"

#include <windows.h>

// For raw input
#include <hidusage.h>

// For device changed events
#include <Dbt.h>

// For the GET_X_LPARAM and GET_Y_LPARAM macros
// https://learn.microsoft.com/en-us/windows/win32/api/windowsx/nf-windowsx-get_x_lparam
#include <windowsx.h>

KeyboardKey::Code GetKeyboardKeyFromVkKey(USHORT virtualKey)
{
	switch (virtualKey)
	{
		case 0x30: return KeyboardKey::Alpha0;
		case 0x31: return KeyboardKey::Alpha1;
		case 0x32: return KeyboardKey::Alpha2;
		case 0x33: return KeyboardKey::Alpha3;
		case 0x34: return KeyboardKey::Alpha4;
		case 0x35: return KeyboardKey::Alpha5;
		case 0x36: return KeyboardKey::Alpha6;
		case 0x37: return KeyboardKey::Alpha7;
		case 0x38: return KeyboardKey::Alpha8;
		case 0x39: return KeyboardKey::Alpha9;

		// Start with letters and numbers that don't have explicit mappings
		case 0x41: return KeyboardKey::A;
		case 0x42: return KeyboardKey::B;
		case 0x43: return KeyboardKey::C;
		case 0x44: return KeyboardKey::D;
		case 0x45: return KeyboardKey::E;
		case 0x46: return KeyboardKey::F;
		case 0x47: return KeyboardKey::G;
		case 0x48: return KeyboardKey::H;
		case 0x49: return KeyboardKey::I;
		case 0x4A: return KeyboardKey::J;
		case 0x4B: return KeyboardKey::K;
		case 0x4C: return KeyboardKey::L;
		case 0x4D: return KeyboardKey::M;
		case 0x4E: return KeyboardKey::N;
		case 0x4F: return KeyboardKey::O;
		case 0x50: return KeyboardKey::P;
		case 0x51: return KeyboardKey::Q;
		case 0x52: return KeyboardKey::R;
		case 0x53: return KeyboardKey::S;
		case 0x54: return KeyboardKey::T;
		case 0x55: return KeyboardKey::U;
		case 0x56: return KeyboardKey::V;
		case 0x57: return KeyboardKey::W;
		case 0x58: return KeyboardKey::X;
		case 0x59: return KeyboardKey::Y;
		case 0x5A: return KeyboardKey::Z;

		case VK_BACK:         return KeyboardKey::Backspace;
		case VK_TAB:          return KeyboardKey::Tab;
		//case VK_CLEAR:      return KeyboardKey::Count;
		case VK_RETURN:       return KeyboardKey::Intro;
		case VK_SHIFT:        return KeyboardKey::LeftShift; // TODO THIS IS WRONG we should never get this as we remapped it
		case VK_CONTROL:      return KeyboardKey::LeftCtrl;
		//case VK_MENU:       return KeyboardKey::Backspace;
		case VK_PAUSE:        return KeyboardKey::Pause;
		case VK_CAPITAL:      return KeyboardKey::CapsLock;
		//case VK_KANA:       return KeyboardKey::Count;
		//case VK_IME_ON:     return KeyboardKey::Count;
		//case VK_JUNJA:      return KeyboardKey::Count;
		//case VK_FINAL:      return KeyboardKey::Count;
		//case VK_KANJI:      return KeyboardKey::Count;
		//case VK_IME_OFF:    return KeyboardKey::Count;
		case VK_ESCAPE:       return KeyboardKey::Escape;
		//case VK_CONVERT:    return KeyboardKey::Count;
		//case VK_NONCONVERT: return KeyboardKey::Count;
		//case VK_ACCEPT:     return KeyboardKey::Count;
		//case VK_MODECHANGE: return KeyboardKey::Count;

		case VK_SPACE:        return KeyboardKey::Space;
		case VK_PRIOR:        return KeyboardKey::Count;
		case VK_NEXT:         return KeyboardKey::Count;
		case VK_END:          return KeyboardKey::End;
		case VK_HOME:         return KeyboardKey::Home;
		case VK_LEFT:         return KeyboardKey::LeftArrow;
		case VK_UP:           return KeyboardKey::UpArrow;
		case VK_RIGHT:        return KeyboardKey::RightArrow;
		case VK_DOWN:         return KeyboardKey::DownArrow;
		//case VK_SELECT:       return KeyboardKey::Count;
		case VK_PRINT:        return KeyboardKey::PrintScreen;
		//case VK_EXECUTE:      return KeyboardKey::Count;
		//case VK_SNAPSHOT:     return KeyboardKey::Count;
		case VK_INSERT:       return KeyboardKey::Insert;
		case VK_DELETE:       return KeyboardKey::Delete;
		//case VK_HELP:         return KeyboardKey::Count;
		case VK_LWIN:         return KeyboardKey::LeftWindowsCmd;
		case VK_RWIN:         return KeyboardKey::RightWindowsCmd;
		//case VK_APPS:         return KeyboardKey::Count;
		//case VK_SLEEP:        return KeyboardKey::Count;

		case VK_NUMPAD0:      return KeyboardKey::Keypad0;
		case VK_NUMPAD1:      return KeyboardKey::Keypad1;
		case VK_NUMPAD2:      return KeyboardKey::Keypad2;
		case VK_NUMPAD3:      return KeyboardKey::Keypad3;
		case VK_NUMPAD4:      return KeyboardKey::Keypad4;
		case VK_NUMPAD5:      return KeyboardKey::Keypad5;
		case VK_NUMPAD6:      return KeyboardKey::Keypad6;
		case VK_NUMPAD7:      return KeyboardKey::Keypad7;
		case VK_NUMPAD8:      return KeyboardKey::Keypad8;
		case VK_NUMPAD9:      return KeyboardKey::Keypad9;
		case VK_MULTIPLY:     return KeyboardKey::KeypadMultiply;
		case VK_ADD:          return KeyboardKey::KeypadPlus;
		//case VK_SEPARATOR:    return KeyboardKey::Keypad0;
		case VK_SUBTRACT:     return KeyboardKey::KeypadMinus;
		//case VK_DECIMAL:      return KeyboardKey::Keypad0;
		//case VK_DIVIDE:       return KeyboardKey::Keypad0;
		case VK_F1:           return KeyboardKey::F1;
		case VK_F2:           return KeyboardKey::F2;
		case VK_F3:           return KeyboardKey::F3;
		case VK_F4:           return KeyboardKey::F4;
		case VK_F5:           return KeyboardKey::F5;
		case VK_F6:           return KeyboardKey::F6;
		case VK_F7:           return KeyboardKey::F7;
		case VK_F8:           return KeyboardKey::F8;
		case VK_F9:           return KeyboardKey::F9;
		case VK_F10:          return KeyboardKey::F10;
		case VK_F11:          return KeyboardKey::F11;
		case VK_F12:          return KeyboardKey::F12;
		case VK_F13:          return KeyboardKey::F13;
		case VK_F14:          return KeyboardKey::F14;
		case VK_F15:          return KeyboardKey::F15;
		case VK_F16:          return KeyboardKey::F16;
		case VK_F17:          return KeyboardKey::F17;
		case VK_F18:          return KeyboardKey::F18;
		case VK_F19:          return KeyboardKey::F19;
		case VK_F20:          return KeyboardKey::F20;
		case VK_F21:          return KeyboardKey::F21;
		case VK_F22:          return KeyboardKey::F22;
		case VK_F23:          return KeyboardKey::F23;
		case VK_F24:          return KeyboardKey::F24;
		case VK_NUMLOCK:      return KeyboardKey::NumLock;
		//case VK_SCROLL:       return KeyboardKey::Keypad0;
		//case VK_OEM_NEC_EQUAL:
		//case VK_OEM_FJ_JISHO:   // 'Dictionary' key
		//case VK_OEM_FJ_MASSHOU:   // 'Unregister word' key
		//case VK_OEM_FJ_TOUROKU:   // 'Register word' key
		//case VK_OEM_FJ_LOYA:   // 'Left OYAYUBI' key
		//case VK_OEM_FJ_ROYA:
		case VK_LSHIFT:       return KeyboardKey::LeftShift;
		case VK_RSHIFT:       return KeyboardKey::RightShift;
		case VK_LCONTROL:     return KeyboardKey::LeftCtrl;
		case VK_RCONTROL:     return KeyboardKey::RightCtrl;
		case VK_LMENU:        return KeyboardKey::Alt;
		case VK_RMENU:        return KeyboardKey::AltGr;
		//case VK_BROWSER_BACK:
		//case VK_BROWSER_FORWARD:
		//case VK_BROWSER_REFRESH:
		//case VK_BROWSER_STOP:
		//case VK_BROWSER_SEARCH:
		//case VK_BROWSER_FAVORITES:
		//case VK_BROWSER_HOME:

		//case VK_VOLUME_MUTE:
		//case VK_VOLUME_DOWN:
		//case VK_VOLUME_UP:
		//case VK_MEDIA_NEXT_TRACK:
		//case VK_MEDIA_PREV_TRACK:
		//case VK_MEDIA_STOP:
		//case VK_MEDIA_PLAY_PAUSE:
		//case VK_LAUNCH_MAIL:
		//case VK_LAUNCH_MEDIA_SELECT:
		//case VK_LAUNCH_APP1:
		//case VK_LAUNCH_APP2:

		//case VK_OEM_1:   // ';:' for US
		//case VK_OEM_PLUS:   // '+' any country
		//case VK_OEM_COMMA:   // ',' any country
		//case VK_OEM_MINUS:   // '-' any country
		//case VK_OEM_PERIOD:   // '.' any country
		//case VK_OEM_2:   // '/?' for US
		//case VK_OEM_3:   // '`~' for US
		//case VK_OEM_4:  //  '[{' for US
		//case VK_OEM_5:  //  '\|' for US
		//case VK_OEM_6:  //  ']}' for US
		//case VK_OEM_7:  //  ''"' for US
		//case VK_OEM_8:
		//case VK_OEM_AX:  //  'AX' key on Japanese AX kbd
		//case VK_OEM_102:  //  "<>" or "\|" on RT 102-key kbd.
		//case VK_ICO_HELP:  //  Help key on ICO
		//case VK_ICO_00:  //  00 key on ICO
		//case VK_PROCESSKEY:
		//case VK_ICO_CLEAR:
		//case VK_PACKET:
		default:
			CrAssertMsg(false, "Key not handled");
			return KeyboardKey::Count;
	}
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CrOSWindow* osWindow = (CrOSWindow*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	LRESULT wndProcResult = 0;

	if (osWindow)
	{
		switch (msg)
		{
			// -----------
			// Mouse Input
			// -----------

			// Captures absolute position of mouse, respecting mouse ballistics
			case WM_MOUSEMOVE:
			{
				POINT clientPosition = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

				POINT absolutePosition = clientPosition;
				::ClientToScreen(hWnd, &absolutePosition);

				CrInput.OnMouseMove(absolutePosition.x, absolutePosition.y, clientPosition.x, clientPosition.y);
				break;
			}
			case WM_NCMOUSEMOVE:
			{
				POINT absolutePosition = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

				POINT clientPosition = absolutePosition;
				::ScreenToClient(hWnd, &clientPosition);

				CrInput.OnMouseMove(absolutePosition.x, absolutePosition.y, clientPosition.x, clientPosition.y);
				break;
			}
			case WM_LBUTTONDBLCLK:
			case WM_LBUTTONDOWN:
			{
				CrInput.OnMouseButtonDown(MouseButton::Left);
				SetCapture(hWnd);
				break;
			}
			case WM_LBUTTONUP:
			{
				CrInput.OnMouseButtonUp(MouseButton::Left);
				ReleaseCapture();
				break;
			}
			case WM_RBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			{
				CrInput.OnMouseButtonDown(MouseButton::Right);
				SetCapture(hWnd);
				break;
			}
			case WM_RBUTTONUP:
			{
				CrInput.OnMouseButtonUp(MouseButton::Right);
				ReleaseCapture();
				break;
			}
			case WM_MBUTTONDBLCLK:
			case WM_MBUTTONDOWN:
			{
				CrInput.OnMouseButtonDown(MouseButton::Middle);
				SetCapture(hWnd);
				break;
			}
			case WM_MBUTTONUP:
			{
				CrInput.OnMouseButtonUp(MouseButton::Middle);
				ReleaseCapture();
				break;
			}
			case WM_XBUTTONDOWN:
			{
				CrInput.OnMouseButtonDown(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MouseButton::X1 : MouseButton::X2);
				break;
			}
			case WM_XBUTTONUP:
			{
				CrInput.OnMouseButtonUp(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MouseButton::X1 : MouseButton::X2);
				break;
			}
			case WM_MOUSEWHEEL:
			{
				CrInput.OnMouseWheelY((float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
				break;
			}
			case WM_MOUSEHWHEEL:
			{
				CrInput.OnMouseWheelX((float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
				break;
			}
			case WM_CHAR:
			{
				// TODO This is for typing into a box, etc
				//InputProducer::RegisterCharInput(this, (uint16_t)wParam);
				break;
			}
			case WM_INPUT:
			{
				UINT dwSize = sizeof(RAWINPUT);
				RAWINPUT rawInput;

				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, (BYTE*)&rawInput, &dwSize, sizeof(RAWINPUTHEADER));

				if (rawInput.header.dwType == RIM_TYPEMOUSE)
				{
					if (!(rawInput.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
					{
						CrInput.OnMouseRelativeMove(rawInput.data.mouse.lLastX, rawInput.data.mouse.lLastY);
					}
				}
				else if (rawInput.header.dwType == RIM_TYPEKEYBOARD)
				{
					USHORT virtualKey      = rawInput.data.keyboard.VKey;
					USHORT scanCode        = rawInput.data.keyboard.MakeCode;
					USHORT flags           = rawInput.data.keyboard.Flags;
					const bool isExtended0 = ((flags & RI_KEY_E0) != 0);
					const bool isExtended1 = ((flags & RI_KEY_E1) != 0);
					bool pressed           = (flags & RI_KEY_BREAK) == 0;

					if (virtualKey == 255)
					{
						// Discard "fake keys" which are part of an escaped sequence
						break;
					}
					else if (virtualKey == VK_SHIFT)
					{
						// Correct left-hand / right-hand Shift
						virtualKey = (USHORT)MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
					}
					else if (virtualKey == VK_NUMLOCK)
					{
						// Correct PAUSE/BREAK and NUM LOCK, and set the extended bit
						scanCode = ((USHORT)MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC) | 0x100);
					}

					if (isExtended1)
					{
						// For escaped sequences, turn the virtual key into the correct scan code using MapVirtualKey.
						// however, MapVirtualKey is unable to map VK_PAUSE (this is a known bug), hence we map that by hand.
						if (virtualKey == VK_PAUSE)
						{
							scanCode = 0x45;
						}
						else
						{
							scanCode = (USHORT)MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
						}
					}

					switch (virtualKey)
					{
						case VK_CONTROL:
							virtualKey = isExtended0 ? VK_RCONTROL : VK_LCONTROL;
							break;
						case VK_MENU:
							virtualKey = isExtended0 ? VK_RMENU : VK_LMENU;
							break;
						case VK_RETURN:
							if (isExtended0) virtualKey = VK_SEPARATOR; // TODO What to do with this
							break;
						case VK_INSERT:
							if (!isExtended0) virtualKey = VK_NUMPAD0;
							break;
						case VK_DELETE:
							if (!isExtended0) virtualKey = VK_DECIMAL;
							break;
						case VK_HOME:
							if (!isExtended0) virtualKey = VK_NUMPAD7;
							break;
						case VK_END:
							if (!isExtended0) virtualKey = VK_NUMPAD1;
							break;
						case VK_PRIOR:
							if (!isExtended0) virtualKey = VK_NUMPAD9;
							break;
						case VK_NEXT:
							if (!isExtended0) virtualKey = VK_NUMPAD3;
							break;
						case VK_LEFT:
							if (!isExtended0) virtualKey = VK_NUMPAD4;
							break;
						case VK_RIGHT:
							if (!isExtended0) virtualKey = VK_NUMPAD6;
							break;
						case VK_UP:
							if (!isExtended0) virtualKey = VK_NUMPAD8;
							break;
						case VK_DOWN:
							if (!isExtended0) virtualKey = VK_NUMPAD2;
							break;
						case VK_CLEAR:
							if (!isExtended0) virtualKey = VK_NUMPAD5;
							break;
					}

					KeyboardKey::Code keyboardKey = GetKeyboardKeyFromVkKey(virtualKey);

					if (pressed)
					{
						CrInput.OnKeyboardDown(keyboardKey);
					}
					else
					{
						CrInput.OnKeyboardUp(keyboardKey);
					}
				}

				break;
			}
			case WM_SIZE:
			{
				WORD sizeX = LOWORD(lParam);
				WORD sizeY = HIWORD(lParam);

				if (sizeX > 0 && sizeY > 0 && osWindow->GetSwapchain())
				{
					osWindow->GetSwapchain()->Resize(sizeX, sizeY);
				}

				break;
			}
			case WM_CLOSE:
			{
				// TODO MSDN page inspect
				break;
			}
			case WM_DESTROY:
			{
				if (osWindow)
				{
					osWindow->Destroy();
				}
				return 0;
			}
			case WM_DEVICECHANGE:
			{
				switch (wParam)
				{
					case DBT_DEVICEARRIVAL:
					case DBT_DEVNODES_CHANGED:
					{
						PlatformInput->RequestRefreshControllers();
						break;
					}
				}
				break;
			}
			case WM_MOVE:              // 3
			case WM_ACTIVATE:          // 6
			case WM_SETFOCUS:          // 7
			case WM_KILLFOCUS:         // 8
			case WM_PAINT:             // 15
			case WM_ERASEBKGND:        // 20
			case WM_SHOWWINDOW:        // 24
			case WM_ACTIVATEAPP:       // 28
			case WM_SETCURSOR:         // 32
			case WM_WINDOWPOSCHANGING: // 70
			case WM_WINDOWPOSCHANGED:  // 71
			case WM_CONTEXTMENU:       // 123
			case WM_GETICON:           // 127
			case WM_NCHITTEST:         // 132
			case WM_NCPAINT:           // 133
			case WM_NCACTIVATE:        // 134
			case WM_SYNCPAINT:         // 136
			case WM_KEYFIRST:          // 256
			case WM_KEYUP:             // 257
			case WM_IME_SETCONTEXT:    // 641
			case WM_IME_NOTIFY:        // 642
			case WM_NCMOUSELEAVE:      // 674
			case WM_DWMNCRENDERINGCHANGED: // 799
			{
				break;
			}
			default:
			{
				//CrLog("Unhandled message %i", msg);
			}
		}
	}

	if (wndProcResult == 0)
	{
		wndProcResult = ::DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	return wndProcResult;
}

WNDCLASSEXW WndClass;
HINSTANCE ModuleHandle;

static LPCWSTR szClassName = L"Corsair Engine";

CrOSWindow::CrOSWindow(const CrOSWindowDescriptor& windowDescriptor)
{
	m_style = WS_OVERLAPPEDWINDOW;

	if (!windowDescriptor.decoration)
	{
		m_style = WS_POPUP;
	}

	m_exStyle = 0;

	if (windowDescriptor.parentWindow)
	{
		m_parentHwnd = windowDescriptor.parentWindow->GetNativeWindowHandle();
	}

	HINSTANCE moduleHandle = ::GetModuleHandle(nullptr);

	LONG viewportX = windowDescriptor.positionX;
	LONG viewportY = windowDescriptor.positionY;

	RECT rect = { viewportX, viewportY, (LONG)(viewportX + windowDescriptor.width), (LONG)(viewportY + windowDescriptor.height) };
	::AdjustWindowRectEx(&rect, (DWORD)m_style, false, (DWORD)m_exStyle);

	m_hwnd = ::CreateWindowExW
	(
		(DWORD)m_exStyle,
		szClassName,
		L"Corsair Engine 0.01",
		(DWORD)m_style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		(HWND)m_parentHwnd,
		nullptr,
		moduleHandle,
		nullptr
	);

	m_hInstance = moduleHandle;

	CrAssert(m_hwnd != nullptr);
	CrAssert(m_hInstance != nullptr);

	::SetWindowLongPtr((HWND)m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

	::ShowWindow((HWND)m_hwnd, SW_SHOW);

	::UpdateWindow((HWND)m_hwnd);

	if (windowDescriptor.swapchainFormat != cr3d::DataFormat::Invalid)
	{
		CrSwapchainDescriptor swapchainDescriptor = {};
		swapchainDescriptor.name = windowDescriptor.name.c_str();
		swapchainDescriptor.window = this;
		swapchainDescriptor.requestedWidth = windowDescriptor.width;
		swapchainDescriptor.requestedHeight = windowDescriptor.height;
		swapchainDescriptor.format = windowDescriptor.swapchainFormat;
		swapchainDescriptor.requestedBufferCount = 3;
		m_swapchain = ICrRenderSystem::GetRenderDevice()->CreateSwapchain(swapchainDescriptor);
		m_swapchain->AcquireNextImage();
	}
}

CrOSWindow::~CrOSWindow()
{
	Destroy();
}

HCURSOR CursorMappingTable[(uint32_t)CursorType::Count] = {};

void CrOSWindow::Initialize()
{
	CrAssert(ModuleHandle == nullptr);

	ModuleHandle           = ::GetModuleHandle(nullptr);

	WndClass.cbSize        = sizeof(WNDCLASSEXA);
	WndClass.style         = CS_HREDRAW | CS_VREDRAW; // Blanks out all window pixels while resizing the window
	WndClass.lpfnWndProc   = WndProc;
	WndClass.cbClsExtra    = 0;
	WndClass.cbWndExtra    = 0;
	WndClass.hInstance     = ModuleHandle;
	WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WndClass.lpszMenuName  = nullptr;
	WndClass.lpszClassName = szClassName;
	WndClass.hCursor       = ::LoadCursor(NULL, IDC_ARROW);

	if (!RegisterClassExW(&WndClass))
	{
		::MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// Register for WM_INPUT messages
	{
		RAWINPUTDEVICE devices[2];

		// Keyboard
		devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[0].usUsage     = HID_USAGE_GENERIC_KEYBOARD;
		devices[0].dwFlags     = 0; // RIDEV_NOLEGACY;
		devices[0].hwndTarget  = nullptr; // Follow keyboard focus

		// Mouse
		devices[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[1].usUsage     = HID_USAGE_GENERIC_MOUSE;
		devices[1].dwFlags     = 0; // RIDEV_NOLEGACY;
		devices[1].hwndTarget  = nullptr; // Follow keyboard focus

		::RegisterRawInputDevices(devices, ARRAYSIZE(devices), sizeof(RAWINPUTDEVICE));
	}

	// Cursors
	{
		CursorMappingTable[CursorType::Arrow]      = ::LoadCursor(nullptr, IDC_ARROW);
		CursorMappingTable[CursorType::TextInput]  = ::LoadCursor(nullptr, IDC_IBEAM);
		CursorMappingTable[CursorType::ResizeAll]  = ::LoadCursor(nullptr, IDC_SIZEALL);
		CursorMappingTable[CursorType::ResizeEW]   = ::LoadCursor(nullptr, IDC_SIZEWE);
		CursorMappingTable[CursorType::ResizeNS]   = ::LoadCursor(nullptr, IDC_SIZENS);
		CursorMappingTable[CursorType::ResizeNESW] = ::LoadCursor(nullptr, IDC_SIZENESW);
		CursorMappingTable[CursorType::ResizeNWSE] = ::LoadCursor(nullptr, IDC_SIZENWSE);
		CursorMappingTable[CursorType::Hand]       = ::LoadCursor(nullptr, IDC_HAND);
		CursorMappingTable[CursorType::NotAllowed] = ::LoadCursor(nullptr, IDC_NO);
		CursorMappingTable[CursorType::None]       = nullptr;
	}
}

void CrOSWindow::SetCursor(CursorType::T cursorType)
{
	::SetCursor(CursorMappingTable[cursorType]);
}

void CrOSWindow::Destroy()
{
	if (m_hwnd)
	{
		::DestroyWindow((HWND)m_hwnd);
		m_hwnd = nullptr;
	}
}

bool CrOSWindow::GetIsMinimized() const
{
	// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-isiconic?redirectedfrom=MSDN
	return ::IsIconic((HWND)m_hwnd);
}

bool CrOSWindow::GetIsFocused() const
{
	return ::GetFocus() == (HWND)m_hwnd;
}

bool CrOSWindow::GetIsDestroyed() const
{
	return m_hwnd == nullptr;
}

void* CrOSWindow::GetNativeWindowHandle() const
{
	return m_hwnd;
}

void* CrOSWindow::GetParentWindowHandle() const
{
	return m_parentHwnd;
}

void* CrOSWindow::GetNativeInstanceHandle() const
{
	return m_hInstance;
}

void CrOSWindow::GetPosition(uint32_t& positionX, uint32_t& positionY) const
{
	POINT pos = { 0, 0 };
	::ClientToScreen((HWND)m_hwnd, &pos);
	positionX = pos.x;
	positionY = pos.y;
}

void CrOSWindow::GetSizePixels(uint32_t& width, uint32_t& height) const
{
	RECT rect;
	::GetClientRect((HWND)m_hwnd, &rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
}

void CrOSWindow::SetPosition(uint32_t positionX, uint32_t positionY)
{
	RECT rect = { (LONG)positionX, (LONG)positionY, (LONG)positionX, (LONG)positionY };
	::AdjustWindowRectEx(&rect, (DWORD)m_style, FALSE, (DWORD)m_exStyle);
	::SetWindowPos((HWND)m_hwnd, nullptr, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

void CrOSWindow::SetSizePixels(uint32_t width, uint32_t height)
{
	RECT rect = { 0, 0, (LONG)width, (LONG)height };
	::AdjustWindowRectEx(&rect, (DWORD)m_style, FALSE, (DWORD)m_exStyle); // Client to Screen
	::SetWindowPos((HWND)m_hwnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

void CrOSWindow::SetFocus()
{
	::SetFocus((HWND)m_hwnd);
}

void CrOSWindow::SetTitle(const char* title)
{
	::SetWindowTextA((HWND)m_hwnd, title);
}

void CrOSWindow::SetTransparencyAlpha(float alpha)
{
	CrAssert(alpha > 0.0f && alpha <= 1.0f);

	if (alpha < 1.0f)
	{
		DWORD exStyle = ::GetWindowLongW((HWND)m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
		::SetWindowLongW((HWND)m_hwnd, GWL_EXSTYLE, exStyle);
		::SetLayeredWindowAttributes((HWND)m_hwnd, 0, (BYTE)(255 * alpha), LWA_ALPHA);
	}
	else
	{
		DWORD exStyle = ::GetWindowLongW((HWND)m_hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
		::SetWindowLongW((HWND)m_hwnd, GWL_EXSTYLE, exStyle);
	}
}

void CrOSWindow::Show()
{
	::ShowWindow((HWND)m_hwnd, SW_NORMAL);
}
