#pragma once

#include <EASTL\string.h>
#include "Schema\typedefs.h"

using namespace eastl;

// The physical representation of a scalar type
struct Type
{
	size_t Size;
	int (*Compare)(void* left, void* right);
	fstring (*ToString)(void* item);
	void (*Free)(void* );
};