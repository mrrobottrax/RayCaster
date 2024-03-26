#pragma once

enum RKeys
{
	IN_BAD_KEY = 0,

	IN_KEY_SPACE,
	IN_KEY_SHIFT,

	IN_ARROW_LEFT,
	IN_ARROW_RIGHT,
	IN_ARROW_UP,
	IN_ARROW_DOWN,

	IN_LETTERS_START = 65,
	IN_LETTERS_END = 90,

	IN_MAX_BUTTONS
};

inline bool keys[IN_MAX_BUTTONS];