#pragma once

struct ColumnRange
{
	int ColumnNumber;
	Range Range;
};

typedef eastl::vector<ColumnRange> Ranges;
