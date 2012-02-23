#pragma once

#include "../Range.h"

struct GetResult
{
	GetResult() : Limited(false) {}
	bool Limited;
	eastl::vector<eastl::pair<void*,void*>> Data;
};

class ColumnBuffer
{
	public:
		virtual void* GetValue(void* rowId) = 0;
		virtual void* Include(void* value, void* rowID) = 0;
		virtual void* Exclude(void* value, void* rowID) = 0;
		virtual GetResult GetRows(Range) = 0;
};
