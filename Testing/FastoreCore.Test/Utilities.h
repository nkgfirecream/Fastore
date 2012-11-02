#pragma once
#include "stdafx.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <string>
using std::string;

#include "..\FastoreCommon\Buffer\IColumnBuffer.h"

void AssignString(string& str, int64_t value);
void AssignBound(RangeBound& bound, bool inclusive, string& value);
void AssignRange(RangeRequest& range, bool ascending, int limit, RangeBound* start = NULL, RangeBound* end = NULL, string* rowId = NULL);





