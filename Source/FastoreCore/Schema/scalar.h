#pragma once

#include "..\typedefs.h"

// The physical representation of a scalar type
struct ScalarType
{
	typedef int (*CompareFunc)(const void* left, const void* right);
	typedef int (*IndexOfFunc)(const char* items, const int count, void *key);
	typedef void (*FreeFunc)(void*);
	typedef fs::wstring (*ToStringFunc)(const void* item);

	size_t Size;
	size_t (*Hash)(const void* item);
	size_t operator ()(const void* item) const
	{
		return Hash(item);
	}
	
	ToStringFunc ToString;
	FreeFunc Free;
	CompareFunc Compare;
	IndexOfFunc IndexOf;

	bool operator ()(const void* left, const void* right) const
	{
		return Compare(left, right) <= 0;
	}
};