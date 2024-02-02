#pragma once
#include "Raycast.h"
#include "Walltype.h"

RaycastResult CastRay(const Ray&);
Vector2Int GetGridPos(const Vector2&);
WallType GetGridType(const Vector2Int&);