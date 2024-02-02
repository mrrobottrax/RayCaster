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

void GameFrame()
{
	static int counter = 0;

	const auto time = std::chrono::system_clock::now();
	localPlayer.Update();
	RenderFrame(localPlayer.camera);
	const auto time2 = std::chrono::system_clock::now();

	const auto dt = time2 - time;

	if (counter > 1000)
	{
		counter = 0;
		std::cout << 1000000 / (float)std::chrono::duration_cast<std::chrono::microseconds>(dt).count() << std::endl;
	}

	++counter;
}
