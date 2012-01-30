#pragma once

#include <string>

using namespace std;

// The physical representation of a scalar type
struct type
{
	size_t Size;
	int (*Compare)(void* left, void* right);
	wstring (*ToString)(void* item);
	void (*Free)(void* );
};