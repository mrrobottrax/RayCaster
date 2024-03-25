#include "pch.h"
#include "game.h"
#include <rendering/software/SoftwareRendering.h>

void InitGame()
{
#ifdef USE_SOFTWARE_RENDERER
	InitSoftwareRendering();
#endif // USE_SOFTWARE_RENDER
}

void CloseGame()
{
#ifdef USE_SOFTWARE_RENDERER
	CloseSoftwareRendering();
#endif // USE_SOFTWARE_RENDER
}

std::chrono::system_clock::time_point t2 = std::chrono::system_clock::now();
float average = 0;
void GameFrame()
{
	localPlayer.Update();

	static int counter = 0;
	const auto time = std::chrono::system_clock::now();
	const auto dt = time - t2;
	t2 = time;

	average += 1000000 / (float)std::chrono::duration_cast<std::chrono::microseconds>(dt).count() / 600;

	if (counter > 600)
	{
		counter = 0;
		std::cout << average << std::endl;
		average = 0;
	}

	++counter;
}
