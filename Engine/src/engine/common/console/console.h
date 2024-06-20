#pragma once

API void Print(const char* format, ...);
API void Println(const char* format, ...);
API void Println(const int);
API void Println(const unsigned int);
API void Println(const float);
API void Println(const double);
API inline void Println() { Println(""); };