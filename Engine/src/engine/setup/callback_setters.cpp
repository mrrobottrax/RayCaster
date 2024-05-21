#include "pch.h"
#include "callback_setters.h"

#include "callbacks.h"

void SetPostInitCallback(void(*Callback)())
{
	PostInitCallback = Callback;
}
