#pragma once

#include "../Type/standardtypes.h"
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
	//const ScalarType& ValueType;
	//const ScalarType& RowIDType;
	ScalarType ValueType;
	ScalarType RowIDType;
	BufferType_t BufferType;
	bool Required;

	//Could be constructors.. Just requires rewriting lots of code.
	/*static ColumnDef make_def(int64_t id, std::string& name, std::string& valueType, std::string& rowType, BufferType_t bufferType, bool required)
	{
		return ColumnDef::make_def(id, name, standardtypes::GetTypeFromName(valueType), standardtypes::GetTypeFromName(rowType), bufferType, required);
	}

	static ColumnDef make_def(int64_t id, std::string& name, const ScalarType& valueType, const ScalarType& rowType, BufferType_t bufferType, bool required)
	{
		return ColumnDef() { id, name, valueType, rowType, bufferType, required };
	}*/
};
