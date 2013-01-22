#pragma once

#include "../Type/standardtypes.h"
#include <stdint.h>
#include "BufferType.h"

struct ColumnDef
{
	int64_t ColumnID;
	std::string Name;
	std::string ValueTypeName;
	std::string RowIDTypeName;
	BufferType_t BufferType;
	bool Required;
};
