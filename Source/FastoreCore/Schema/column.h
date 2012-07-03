#pragma once

#include "scalar.h"

enum BufferType
{
	Identity = 0,
	Unique = 1,
	Multi = 2
};

struct ColumnDef
{
	int ColumnID;
	std::string Name;
	ScalarType ValueType;
	ScalarType RowIDType;
	BufferType BufferType;
};
