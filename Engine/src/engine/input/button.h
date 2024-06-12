#pragma once
#include "keys.h"

enum Button
{
	BUTTON_NONE,

	BUTTON_FORWARD,
	BUTTON_BACK,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_UP,
	BUTTON_DOWN,

	BUTTON_LOOK_LEFT,
	BUTTON_LOOK_RIGHT,
	BUTTON_LOOK_UP,
	BUTTON_LOOK_DOWN,
};

Button GetBoundButton(KeyCode);

void SetButtonDown(Button);
void SetButtonUp(Button);
bool GetButtonDown(Button);