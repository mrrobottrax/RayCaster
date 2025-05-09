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

	BUTTON_PLACE,
	BUTTON_BREAK,

	BUTTON_ITEM1,
	BUTTON_ITEM2,
	BUTTON_ITEM3,
	BUTTON_ITEM4,
	BUTTON_ITEM5,

	TOGGLE_HUD,
};

Button GetBoundButton(KeyCode);

void SetButtonDown(Button);
void SetButtonUp(Button);

// Returns true when a button was pressed this frame
bool GetButtonPressed(Button);
bool GetButtonDown(Button);

void EndOfFrameButtons();