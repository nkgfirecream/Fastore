#pragma once

#include "scalar.h"

struct ColumnDef
{
	std::string Name;
	ScalarType ValueType;
	ScalarType RowIDType;
	bool IsUnique;
	int ColumnID;
};
