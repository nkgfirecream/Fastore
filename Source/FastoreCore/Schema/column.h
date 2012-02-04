#pragma once

#include "scalar.h"
#include "..\typedefs.h"

struct ColumnType
{
	const fs::wstring Name;
	const ScalarType Type;

	ColumnType(const ScalarType type, const fs::wstring name) : Type(type), Name(name) {};
}

