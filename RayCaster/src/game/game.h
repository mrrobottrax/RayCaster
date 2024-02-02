#pragma once
#include <player/Player.h>

inline Player localPlayer({0.5, 0.5});

void InitGame();
void CloseGame();
void GameFrame();