#pragma once
#include "Raycast.h"

typedef BYTE WallContents;

RaycastResult CastRay(Ray&);
Vector2Int GetGridPos(Vector2&);
WallContents GetWallContents(Vector2Int&);