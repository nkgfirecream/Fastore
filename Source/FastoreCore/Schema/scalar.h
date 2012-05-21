#pragma once

#include "..\typedefs.h"

// The physical representation of a scalar type
struct ScalarType
{
	typedef int (*CompareFunc)(const void* left, const void* right);
	typedef int (*IndexOfFunc)(const char* items, const int count, void *key);
	typedef void (*FreeFunc)(void*);
	typedef fs::wstring (*ToStringFunc)(const void* item);
	typedef void (*CopyInFunc)(const void* item, void* arraypointer);
	typedef void (*EncodeFunc)(const void*, std::string&);
	typedef void (*DecodeFunc)(const std::string&, void*&);
	typedef void* (*AllocateFunc)();

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
	CopyInFunc CopyIn;
	fs::string Name;
	EncodeFunc Encode;
	DecodeFunc Decode;
	AllocateFunc Allocate;

	bool operator ()(const void* left, const void* right) const
	{
		return Compare(left, right) <= 0;
	}
};