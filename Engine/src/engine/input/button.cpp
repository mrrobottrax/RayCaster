#include "pch.h"
#include "button.h"

std::map buttons = std::map<Button, bool>();

Button GetBoundButton(KeyCode code)
{
	switch (code)
	{
	// Movement
	case KEY_W:
		return BUTTON_FORWARD;
	case KEY_A:
		return BUTTON_LEFT;
	case KEY_S:
		return BUTTON_BACK;
	case KEY_D:
		return BUTTON_RIGHT;

	// Looking
	case KEY_ARROW_LEFT:
		return BUTTON_LOOK_LEFT;
	case KEY_ARROW_RIGHT:
		return BUTTON_LOOK_RIGHT;
	case KEY_ARROW_UP:
		return BUTTON_LOOK_UP;
	case KEY_ARROW_DOWN:
		return BUTTON_LOOK_UP;

	default:
		return BUTTON_NONE;
	}
}

void SetButtonDown(Button button)
{
	buttons[button] = true;
}

void SetButtonUp(Button button)
{
	buttons[button] = false;
}


bool GetButtonDown(Button button)
{
	if (!buttons.contains(button)) return false;

	return buttons[button];
}