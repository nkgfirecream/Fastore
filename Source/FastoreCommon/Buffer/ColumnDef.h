#pragma once

#include "../Type/standardtypes.h"
#include <stdint.h>
#include "BufferType.h"

struct ColumnDef
{
	int64_t ColumnID;
	std::string Name;
	//const ScalarType& ValueType;
	//const ScalarType& RowIDType;
	std::string ValueTypeName;
	std::string RowIDTypeName;
	BufferType_t BufferType;
	bool Required;
};
