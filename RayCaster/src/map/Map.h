#pragma once
#include "Raycast.h"
#include "Walltype.h"

constexpr int mapWidth = 5;
constexpr int mapHeight = 5;

RaycastResult CastRay(const Ray&);
Vector2Int GetGridPos(const Vector2&);
Vector2Int GetGridPos(const Vector3&);
WallType GetGridType(const Vector2Int&);
const WallType* GetMapPointer();