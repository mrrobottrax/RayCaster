#pragma once
#include <player/Player.h>

inline Player localPlayer{
	{
		{
			0.5f, 0.5f
		}
	}
};

void InitGame();
void CloseGame();
void GameFrame();