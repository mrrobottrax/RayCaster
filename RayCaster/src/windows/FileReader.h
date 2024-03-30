#pragma once

struct File
{
	const size_t length;
	const char* data;
};

const File ReadFileFull(const char* path);