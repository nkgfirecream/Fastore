#pragma once

#include "scalar.h"

struct ColumnDef
{
	fs::wstring Name;
	ScalarType KeyType;
	ScalarType IDType;
	bool IsUnique;
	bool IsRequired;
};
