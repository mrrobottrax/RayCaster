#pragma once

enum RButton
{
	IN_BAD_KEY,

	IN_LETTERS_START = 65, // Capital letters
	IN_LETTERS_END = 90,

	IN_ARROW_LEFT,
	IN_ARROW_RIGHT,
	IN_ARROW_UP,
	IN_ARROW_DOWN,

	IN_MAX_BUTTONS
};

inline bool buttons[IN_MAX_BUTTONS];