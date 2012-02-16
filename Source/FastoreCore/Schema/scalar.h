#pragma once

#include "..\typedefs.h"

// The physical representation of a scalar type
struct ScalarType
{
	size_t Size;
	size_t (*Hash)(const void* item);
	size_t operator ()(const void* item) const
	{
		return Hash(item);
	}
	
	fs::wstring (*ToString)(const void* item);
	void (*Free)(void*);

	int (*Compare)(const void* left, const void* right);
	bool operator ()(const void* left, const void* right) const
	{
		return Compare(left,right) <= 0;
	}
};