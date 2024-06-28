#pragma once

namespace Time
{
	inline double dDeltaTime;
	inline float deltaTime;
	inline std::chrono::nanoseconds deltaNano;

	constexpr int tps = 256;
	constexpr double dTickDeltaTime = 1.0 / tps;
	constexpr float tickDeltaTime = 1.0 / tps;
}

void UpdateDeltaTime();
void TryTick();