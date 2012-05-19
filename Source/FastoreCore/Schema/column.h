#pragma once

#include "scalar.h"

struct ColumnDef
{
	fs::string Name;
	ScalarType ValueType;
	ScalarType RowIDType;
	bool IsUnique;
	int ColumnID;
};
