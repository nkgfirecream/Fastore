#pragma once

#include "../Range.h"

struct ColumnRange
{
	int ColumnNumber;
	Range Range;
};

typedef eastl::vector<ColumnRange> Ranges;
