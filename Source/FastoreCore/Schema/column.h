#pragma once

#include "scalar.h"

struct ColumnType
{
	fs::wstring Name;
	ScalarType Type;
	bool IsUnique;
	bool IsRequired;
};

