#include "pch.h"

#include <input/keys.h>
#include "w_input.h"

KeyCode W_TranslateToKeyCode(WPARAM code)
{
	if (code >= KEY_A && code <= KEY_Z)
	{
		return static_cast<KeyCode>(code);
	}

	return (KeyCode)0;
}