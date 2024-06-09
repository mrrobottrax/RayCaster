#pragma once

#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS

// std
#include "stdlib.h"
#include "crtdbg.h"
#include "assert.h"
#include "stdio.h"
#include "iostream"
#include "string"
#include "chrono"
#include "ctime" 
#include "thread"
#include "random"
#include "map"
#include "optional"
#include "set"
#include "fstream"
#include "filesystem"
#include "array"

#undef _CRT_SECURE_NO_WARNINGS

// Platform
#include "platform.h"

// API
#include "api.h"

// Common
#include "common/console/console.h"
#include "common/local_array.h"
#include "common/math.h"
#include "common/mat/mat.h"
#include "common/vec/vec.h"