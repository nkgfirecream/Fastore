#pragma once

#include "../Range.h"

struct GetResult
{
	bool Limited;
	eastl::vector<eastl::pair<void*,void*>> Data;
};

class ColumnBuffer
{
	public:
		virtual void* GetValue(void* value) = 0;
		virtual void* Include(void* value, void* rowID) = 0;
		virtual void* Exclude(void* value, void* rowID) = 0;
		// TODO: what should updatevalue return?  All the row IDs?
		virtual void UpdateValue(void* oldValue, void* newValue) = 0;
		virtual GetResult GetRows(Range) = 0;
};
