#include "pch.h"

#include <input/keys.h>
#include "w_input.h"
#include "hidusage.h"
#include <_platform/windows/window/W_Window.h>

void W_InitInput()
{
	RAWINPUTDEVICE rid[1]{};

	rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	rid[0].dwFlags = 0; // RIDEV_CAPTUREMOUSE
	rid[0].hwndTarget = W_Window::hWnd;

	if (RegisterRawInputDevices(rid, 1, sizeof(rid[0])) == FALSE)
	{
		throw windows_error("Failed to register raw input");
	}
}

KeyCode W_TranslateToKeyCode(WPARAM code)
{
	if (code >= KEY_A && code <= KEY_Z)
	{
		return static_cast<KeyCode>(code);
	}

	KeyCode outCode = KEY_NONE;

	switch (code)
	{
	case 16:
		outCode = KEY_LSHIFT;
		break;
	case 32:
		outCode = KEY_SPACE;
		break;
	case 37:
		outCode = KEY_ARROW_LEFT;
		break;
	case 38:
		outCode = KEY_ARROW_UP;
		break;
	case 39:
		outCode = KEY_ARROW_RIGHT;
		break;
	case 40:
		outCode = KEY_ARROW_DOWN;
		break;
	default:
		Println("%u", code);
		break;
	}

	return outCode;
}