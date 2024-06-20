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

API void Println(const int n)
{
	Println("%i", n);
}

API void Println(const unsigned int n)
{
	Println("%u", n);
}

API void Println(const float n)
{
	Println("%f", n);
}

API void Println(const double n)
{
	Println("%f", n);
}