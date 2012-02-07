#pragma once

#include "scalar.h"
#include "..\typedefs.h"

struct ColumnType
{
	fs::wstring Name;
	ScalarType Type;
	bool IsUnique;
	bool IsRequired;
};

