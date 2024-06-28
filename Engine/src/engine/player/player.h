#pragma once

extern vec3 camPos;
extern vec2 camRot;
extern bool hasSelectedBlock;
extern uvec3 selectedBlock;

void PlayerFrame();
void PlayerTick();