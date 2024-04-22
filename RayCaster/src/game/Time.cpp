#include "pch.h"
#include "Time.h"

std::chrono::system_clock::time_point oldTime = std::chrono::system_clock::now();

void Time::UpdateTime()
{
	static int counter = 0;
	static float average = 0;

	const auto time = std::chrono::system_clock::now();
	const auto dt = time - oldTime;
	oldTime = time;

	deltaTimeD = std::chrono::duration_cast<std::chrono::microseconds>(dt).count() / 1000000.0;
	deltaTime = (float)deltaTimeD;

	average += 1 / deltaTime / 600;

	if (counter > 600)
	{
		counter = 0;
		std::cout << average << std::endl;
		average = 0;
	}

	++counter;
}

double Time::TimeSinceLastUpdate()
{
	const auto time = std::chrono::system_clock::now();
	const auto dt = time - oldTime;

	return std::chrono::duration_cast<std::chrono::microseconds>(dt).count() / 1000000.0;
}
