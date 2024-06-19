#include "pch.h"

#include <input/keys.h>
#include "w_input.h"
#include "hidusage.h"
#include <_platform/windows/window/W_Window.h>
#include <input/mouse.h>

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
	case 27:
		W_ShowCursor();
		outCode = KEY_ESCAPE;
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

POINT lastCursorPos;
bool rawInputOn = false;
void W_HideCursor()
{
	if (rawInputOn) return;

	RAWINPUTDEVICE rid[1]{};

	rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	rid[0].dwFlags = RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE;
	rid[0].hwndTarget = W_Window::hWnd;

	if (RegisterRawInputDevices(rid, (UINT)std::size(rid), sizeof(rid[0])) == FALSE)
	{
		throw windows_error("Failed to register raw input");
	}

	GetCursorPos(&lastCursorPos);

#ifndef DEBUG
	ShowCursor(false);
#endif // DEBUG

	rawInputOn = true;
}

void W_ShowCursor()
{
	if (!rawInputOn) return;

	RAWINPUTDEVICE rid[1]{};

	rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	rid[0].dwFlags = RIDEV_REMOVE;
	rid[0].hwndTarget = NULL;

	if (RegisterRawInputDevices(rid, (UINT)std::size(rid), sizeof(rid[0])) == FALSE)
	{
		throw windows_error("Failed to remove raw input");
	}

	SetCursorPos(lastCursorPos.x, lastCursorPos.y);

#ifndef DEBUG
	ShowCursor(true);
#endif // DEBUG

	rawInputOn = false;
}

void W_RawInputBuffer()
{
	UINT cbSize = 0;

	if (GetRawInputBuffer(NULL, &cbSize, sizeof(RAWINPUTHEADER)) != 0)
	{
		throw windows_error("Error getting raw input buffer size");
	}

	if (cbSize < 0)
	{
		throw windows_error("Error getting raw input buffer size");
	}

	if (cbSize == 0)
	{
		return;
	}

	cbSize *= 16; // up to 16 messages

	PRAWINPUT pRawInput = (PRAWINPUT)malloc(cbSize);
	if (pRawInput == NULL)
	{
		throw std::runtime_error("Not enough memory");
	}

	UINT cbSizeT = cbSize;
	UINT nInput = GetRawInputBuffer(pRawInput, &cbSizeT, sizeof(RAWINPUTHEADER));

	if (nInput < 0)
	{
		throw windows_error("Failed to get raw input buffer");
	}

	if (nInput > 0)
	{
		//Println("nInput = %d", nInput);

		PRAWINPUT pri = pRawInput;
		for (UINT i = 0; i < nInput; ++i)
		{
			//Println(" input[%d] = @%p", i, pri);
			//Println(" type = %u", i, pri->header.dwType);
			if (pri->header.dwType == 0)
			{
				LONG x = pri->data.mouse.lLastX;
				LONG y = pri->data.mouse.lLastY;
				UpdateMouseDelta(x, y);

				const USHORT& flags = pri->data.mouse.usButtonFlags;

				if (flags & RI_MOUSE_BUTTON_1_DOWN) KeyDown(KEY_MOUSE_1);
				if (flags & RI_MOUSE_BUTTON_1_UP) KeyUp(KEY_MOUSE_1);
				if (flags & RI_MOUSE_BUTTON_2_DOWN) KeyDown(KEY_MOUSE_2);
				if (flags & RI_MOUSE_BUTTON_2_UP) KeyUp(KEY_MOUSE_2);
				if (flags & RI_MOUSE_BUTTON_3_DOWN) KeyDown(KEY_MOUSE_3);
				if (flags & RI_MOUSE_BUTTON_3_UP) KeyUp(KEY_MOUSE_3);
				if (flags & RI_MOUSE_BUTTON_4_DOWN) KeyDown(KEY_MOUSE_4);
				if (flags & RI_MOUSE_BUTTON_4_UP) KeyUp(KEY_MOUSE_4);
				if (flags & RI_MOUSE_BUTTON_5_DOWN) KeyDown(KEY_MOUSE_5);
				if (flags & RI_MOUSE_BUTTON_5_UP) KeyUp(KEY_MOUSE_5);
			}
			pri = NEXTRAWINPUTBLOCK(pri);
		}
	}

	free(pRawInput);
}