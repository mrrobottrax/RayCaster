#include "pch.h"
#include "button.h"

void KeyDown(KeyCode code)
{
	SetButtonDown(GetBoundButton(code));
}

void KeyUp(KeyCode code)
{
	SetButtonUp(GetBoundButton(code));
}