#pragma once
#include "Raycast.h"
#include "Walltype.h"

RaycastResult CastRay(const Ray&);
Vector2Int GetGridPos(const Vector2&);
Vector2Int GetGridPos(const Vector3&);
WallType GetGridType(const Vector2Int&);