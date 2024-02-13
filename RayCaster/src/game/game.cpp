#include "pch.h"
#include "game.h"
#include <rendering/Rendering.h>

void InitGame()
{
	InitRendering();
}

void CloseGame()
{
	CloseRendering();
}

std::chrono::system_clock::time_point t2 = std::chrono::system_clock::now();
void GameFrame()
{
	localPlayer.Update();

	static int counter = 0;
	const auto time = std::chrono::system_clock::now();
	const auto dt = time - t2;
	t2 = time;

	if (counter > 300)
	{
		counter = 0;
		std::cout << 1000000 / (float)std::chrono::duration_cast<std::chrono::microseconds>(dt).count() << std::endl;
	}

	++counter;
}
