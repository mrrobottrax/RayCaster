#pragma once
#include "keys.h"

enum Button
{
	BUTTON_NONE,

	BUTTON_FORWARD,
	BUTTON_BACK,
	BUTTON_LEFT,
	BUTTON_RIGHT,
};

Button GetBoundButton(KeyCode);

void SetButtonDown(Button);
void SetButtonUp(Button);
bool GetButtonDown(Button);