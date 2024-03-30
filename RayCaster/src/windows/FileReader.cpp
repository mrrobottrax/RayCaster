#include "pch.h"
#include "FileReader.h"

const File ReadFileFull(const char* path)
{
	HANDLE fileHandle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD size = GetFileSize(fileHandle, nullptr);
	
	assert(size != INVALID_FILE_SIZE);

	void* data = malloc(size);
	DWORD bytesRead;
	bool success = ReadFile(fileHandle, data, size, &bytesRead, nullptr);

	assert(success);

	return {
		size,
		(const char*)data
	};
}