#include "pch.h"
#include "time.h"
#include <game/game.h>

using namespace std::chrono;
using namespace Time;

time_point nextTick = high_resolution_clock::now();
time_point oldTime = high_resolution_clock::now();

void UpdateDeltaTime()
{
	// Delta T
	time_point time = high_resolution_clock::now();
	deltaNano = duration_cast<nanoseconds>(time - oldTime);

	oldTime = time;

	dDeltaTime = deltaNano.count() / 1000000000.0;
	deltaTime = (float)dDeltaTime;

	// Fraction
	const double timeTillTick = duration_cast<nanoseconds>(nextTick - time).count() / 1000000000.0;
	dTickFraction = 1 - timeTillTick / Time::dTickDeltaTime;
	tickFraction = (float)dTickFraction;
}

void TryTick()
{
	time_point now = high_resolution_clock::now();

	while (now > nextTick)
	{
		GameTick();

		nextTick = nextTick + std::chrono::nanoseconds(static_cast<long long>(dTickDeltaTime * 1000000000.0));
		now = high_resolution_clock::now();
	}
}