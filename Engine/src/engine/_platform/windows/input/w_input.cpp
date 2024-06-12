#include "pch.h"

#include <input/keys.h>
#include "w_input.h"

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