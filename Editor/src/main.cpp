#include "pch.h"

void Setup()
{
	SetPostInitCallback([]()
		{
			Println("Game Start!");
		}
	);
}