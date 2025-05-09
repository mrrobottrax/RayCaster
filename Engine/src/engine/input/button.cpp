#include "pch.h"
#include "button.h"

std::map buttons = std::map<Button, bool>();
std::map lastButtons = std::map<Button, bool>();
std::map lastTickButtons = std::map<Button, bool>();

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
	case KEY_SPACE:
		return BUTTON_UP;
	case KEY_LSHIFT:
		return BUTTON_DOWN;

	// Looking
	case KEY_ARROW_LEFT:
		return BUTTON_LOOK_LEFT;
	case KEY_ARROW_RIGHT:
		return BUTTON_LOOK_RIGHT;
	case KEY_ARROW_UP:
		return BUTTON_LOOK_UP;
	case KEY_ARROW_DOWN:
		return BUTTON_LOOK_DOWN;

	// Place / Destroy
	case KEY_J:
		return BUTTON_BREAK;
	case KEY_K:
		return BUTTON_PLACE;
	case KEY_MOUSE_1:
		return BUTTON_BREAK;
	case KEY_MOUSE_2:
		return BUTTON_PLACE;

	// Items
	case KEY_1:
		return BUTTON_ITEM1;
	case KEY_2:
		return BUTTON_ITEM2;
	case KEY_3:
		return BUTTON_ITEM3;
	case KEY_4:
		return BUTTON_ITEM4;
	case KEY_5:
		return BUTTON_ITEM5;
	case KEY_6:
		return BUTTON_ITEM6;
	case KEY_7:
		return BUTTON_ITEM7;
	case KEY_8:
		return BUTTON_ITEM8;
	case KEY_9:
		return BUTTON_ITEM9;
	case KEY_0:
		return BUTTON_ITEM10;

	// Debug
	case KEY_F1:
		return TOGGLE_HUD;

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

bool GetButtonPressed(Button button)
{
	if (!buttons.contains(button)) return false;

	return buttons[button] && !lastButtons[button];
}

bool GetButtonPressedTick(Button button)
{
	if (!buttons.contains(button)) return false;

	return buttons[button] && !lastTickButtons[button];
}

void EndOfFrameButtons()
{
	lastButtons = buttons;
}

void EndOfTickButtons()
{
	lastTickButtons = buttons;
}