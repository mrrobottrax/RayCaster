#pragma once
#include "Raycast.h"
#include "Walltype.h"

RaycastResult CastRay(Ray&);
Vector2Int GetGridPos(Vector2&);
WallType GetWallType(Vector2Int&);