#pragma once

#include "scalar.h"

struct ColumnDef
{
	fs::wstring Name;
	ScalarType ValueType;
	ScalarType RowIDType;
	bool IsUnique;
	bool IsRequired;
	int ColumnID;
};
