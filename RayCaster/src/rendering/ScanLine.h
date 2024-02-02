#pragma once
#include <map/Walltype.h>

struct ScanLine
{
	int wallStart;
	int wallEnd;

	WallType wallType;
	bool northSouth;
	Vector2 hitPos;
};