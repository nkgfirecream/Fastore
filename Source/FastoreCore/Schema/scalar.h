#pragma once

#include "..\typedefs.h"

// The physical representation of a scalar type
struct ScalarType
{
	size_t Size;
	int (*Compare)(void* left, void* right);
	fs::wstring (*ToString)(void* item);
	void (*Free)(void*);
};