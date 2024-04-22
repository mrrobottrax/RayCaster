#pragma once

namespace Time
{
	inline double deltaTimeD;
	inline float deltaTime;

	void UpdateTime();
	double TimeSinceLastUpdate();
}