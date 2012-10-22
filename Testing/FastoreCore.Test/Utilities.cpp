#include "stdafx.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "Utilities.h"

void AssignString(string& str, int64_t value)
{
	str.assign((const char*)&value, sizeof(int64_t));
}

void AssignBound(RangeBound& bound, bool inclusive, string& value)
{
	bound = RangeBound();
	bound.__set_inclusive(inclusive);
	bound.__set_value(value);
}

void AssignRange(RangeRequest& range, bool ascending, int limit, RangeBound* start, RangeBound* end, string* rowId)
{
	range = RangeRequest();
	range.__set_ascending(ascending);
	range.__set_limit(limit);

	if (start != NULL)
		range.__set_first(*start);

	if (end != NULL)
		range.__set_last(*end);

	if (rowId != NULL)
		range.__set_rowID(*rowId);
}