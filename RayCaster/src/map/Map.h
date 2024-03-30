#pragma once
#include "Raycast.h"
#include "Walltype.h"

constexpr int mapWidth = 256;
constexpr int mapHeight = 256;
constexpr int mapDepth = 256;

void LoadMap(const char* path);

RaycastResult CastRay(const Ray&);
Vector3Int GetGridPos(const Vector3&);
WallType GetGridType(const Vector3Int&);
const WallType* GetMapPointer();