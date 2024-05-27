#include "pch.h"
#include "console.h"

void Print(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	vprintf_s(format, args);

	va_end(args);
}

void Println(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	vprintf_s(format, args);
	printf("\n");

	va_end(args);
}