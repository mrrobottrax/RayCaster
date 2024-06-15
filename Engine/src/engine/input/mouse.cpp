#include "pch.h"
#include "mouse.h"

int32_t deltaX = 0, deltaY = 0;

void GetMouseDeltaD(double* pX, double* pY)
{
	*pX = (double)deltaX;
	*pY = (double)deltaY;
}

void UpdateMouseDelta(int32_t x, int32_t y)
{
	deltaX += x;
	deltaY += y;
}

void ResetMouseDelta()
{
	deltaX = 0;
	deltaY = 0;
}