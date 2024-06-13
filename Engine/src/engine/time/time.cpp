#include "pch.h"
#include "time.h"

using namespace std::chrono;
using namespace Time;

time_point oldTime = high_resolution_clock::now();

void UpdateDeltaTime()
{
	time_point time = high_resolution_clock::now();
	deltaNano = duration_cast<nanoseconds>(time - oldTime);

	oldTime = time;

	dDeltaTime = deltaNano.count() / 1000000000.0;
	deltaTime = (float)dDeltaTime;
}