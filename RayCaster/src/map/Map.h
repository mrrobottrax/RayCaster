#pragma once
#include "Raycast.h"
#include "Walltype.h"

constexpr int mapWidth = 5;
constexpr int mapHeight = 5;
constexpr int mapDepth = 5;

RaycastResult CastRay(const Ray&);
Vector3Int GetGridPos(const Vector3&);
WallType GetGridType(const Vector3Int&);
const WallType* GetMapPointer();