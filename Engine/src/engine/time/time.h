#pragma once

namespace Time
{
	inline double dDeltaTime;
	inline float deltaTime;
	inline std::chrono::nanoseconds deltaNano;
}

void UpdateDeltaTime();