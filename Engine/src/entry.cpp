#include "core_pch.h"
#include "entry.h"

#include "entry_private.h"

API void SetEntryCallback(void *callback)
{
    entryCallback = callback;
}
