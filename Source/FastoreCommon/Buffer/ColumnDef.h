#pragma once
#include "..\Type\Scalar.h"
#include <stdint.h>

enum BufferType_t
{
	Identity = 0,
	Unique = 1,
	Multi = 2
};

struct ColumnDef
{
	int64_t ColumnID;
	std::string Name;
	ScalarType ValueType;
	ScalarType RowIDType;
	BufferType_t BufferType;
	bool Required;
};