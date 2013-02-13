#pragma once
#include "stdafx.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "RangeTests.h"

void OneToOneRangeTest(IColumnBuffer* buf)
{
	ColumnWrites cw;
	std::vector<Cell> includes;

	//Insert values 0 - 98 (inclusive) in increments of 2 into buffer
	for (int64_t i = 0; i < 100; i += 2)
	{
		Cell inc;
		//TODO: Create thrift strings
		string rowId;
		AssignString(rowId, i);

		string value;
		AssignString(value, i);

		inc.__set_rowID(rowId);
		inc.__set_value(value);
		includes.push_back(inc); 
	}

	cw.__set_includes(includes);
	buf->Apply(cw);

	Assert::AreEqual<int64_t>(buf->GetStatistic().total, 50);
	Assert::AreEqual<int64_t>(buf->GetStatistic().unique, 50);

	string startv;
	string endv;
	RangeBound startBound;
	RangeBound endBound;
	RangeRequest range;

	//Should BoF/Eof be true if we've returned every value in the requested range, or if
	//we've hit the first/last values in the set?
	/*AssignString(endv, 9);
	AssignBound(endBound, false, endv);
	AssignRange(range, true, 500, NULL, &endBound);
	TestRange<int64_t>(buf, range, 0, 8, 5, 2, true, true, false);*/
		
	//Entire Set
	//Range: Entire set ascending
	//Expected result: values 0 - 98 (inclusive) by 2s.
	AssignRange(range, true, 500);
	TestRange<int64_t>(buf, range, 0, 98, 50, 2, true, true, false);

	//Range: Entire set descending
	//Expected result: values 98 - 0 (inclusive) by -2s.
	AssignRange(range, false, 500);
	TestRange<int64_t>(buf, range, 98, 0, 50, -2, true, true, false);


	//Start Exclusive - Non overlapping
	//Range: 0 (exclusive) - end (inclusive) 
	//Expected result: values 2 - 98 (inclusive)
	AssignString(startv, 0);
	AssignBound(startBound, false, startv);
	AssignRange(range, true, 500, &startBound);
	TestRange<int64_t>(buf, range, 2, 98, 49, 2, false, true, false);

	//Range:  98 (inc) - 0 (exclusive) 
	//Expected result: values 98 - 2 (inclusive)
	AssignString(endv, 0);
	AssignBound(endBound, false, endv);
	AssignRange(range, false, 500, NULL, &endBound);
	TestRange<int64_t>(buf, range, 98, 2, 49, -2, true, false, false);


	//End Exclusive - Non overlapping
	//Range: begin (inclusive) - 98 (exclusive)
	//Expected result: values 0 - 96 (inclusive)
	AssignString(endv, 98);
	AssignBound(endBound, false, endv);
	AssignRange(range, true, 500, NULL, &endBound);
	TestRange<int64_t>(buf, range, 0, 96, 49, 2, true, false, false);

	//Range: 98 (exclusive)  -  0 (inclusive)
	//Expected result: values 96 - 0 (inclusive)
	AssignString(startv, 98);
	AssignBound(startBound, false, startv);
	AssignRange(range, false, 500, &startBound);
	TestRange<int64_t>(buf, range, 96, 0, 49, -2, false, true, false);


	//Two bounds - inclusive, non overlapping
	//Range: 2 (inclusive) - 96 (inclusive)
	//Expected result: values 2 - 96 (inclusive)
	AssignString(endv, 96);
	AssignBound(endBound, true, endv);
	AssignString(startv, 2);
	AssignBound(startBound, true, startv);		
	AssignRange(range, true, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 2, 96, 48, 2, false, false, false);

	//Range: 96 (inclusive) - 2 (inclusive)
	//Expected result: values 96 - 2 (inclusive)
	AssignString(endv, 2);
	AssignBound(endBound, true, endv);
	AssignString(startv, 96);
	AssignBound(startBound, true, startv);		
	AssignRange(range, false, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 96, 2, 48, -2, false, false, false);

	//Range: 3 (inclusive) - 95 (inclusive)
	//Expected result: values 4 - 94 (inclusive)
	AssignString(endv, 95);
	AssignBound(endBound, true, endv);
	AssignString(startv, 3);
	AssignBound(startBound, true, startv);		
	AssignRange(range, true, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 4, 94, 46, 2, false, false, false);

	//Range: 95 (inclusive) - 3 (inclusive)
	//Expected result: values 94 - 4 (inclusive)
	AssignString(endv, 3);
	AssignBound(endBound, true, endv);
	AssignString(startv, 95);
	AssignBound(startBound, true, startv);		
	AssignRange(range, false, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 94, 4, 46, -2, false, false, false);


	//Two bounds - exclusive, non overlapping
	//Range: 2 (exclusive) - 96 (exclusive)
	//Expected result: values 4 - 94 (inclusive)
	AssignString(endv, 96);
	AssignBound(endBound, false, endv);
	AssignString(startv, 2);
	AssignBound(startBound, false, startv);		
	AssignRange(range, true, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 4, 94, 46, 2, false, false, false);

	//Range: 96 (exclusive) - 2 (exclusive)
	//Expected result: values 94 - 4 (inclusive)
	AssignString(endv, 2);
	AssignBound(endBound, false, endv);
	AssignString(startv, 96);
	AssignBound(startBound, false, startv);		
	AssignRange(range, false, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 94, 4, 46, -2, false, false, false);

	//Range: 3 (exclusive) - 95 (exclusive)
	//Expected result: values 4 - 94 (exclusive)
	AssignString(endv, 95);
	AssignBound(endBound, false, endv);
	AssignString(startv, 3);
	AssignBound(startBound, false, startv);		
	AssignRange(range, true, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 4, 94, 46, 2, false, false, false);

	//Range: 95 (exclusive) - 3 (exclusive)
	//Expected result: values 94 - 4 (inclusive)
	AssignString(endv, 3);
	AssignBound(endBound, false, endv);
	AssignString(startv, 95);
	AssignBound(startBound, false, startv);		
	AssignRange(range, false, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 94, 4, 46, -2, false, false, false);


	//Two bounds - inclusive, overlapping
	//Range: 50 (inclusive) - 50 (inclusive) ascending
	//Expected result: 50
	AssignString(endv, 50);
	AssignBound(endBound, true, endv);
	AssignString(startv, 50);
	AssignBound(startBound, true, startv);
	AssignRange(range, true, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 50, 50, 1, 0, false, false, false);

	//Range: 50 (inclusive) - 50 (inclusive) desc
	//Expected result: 50
	AssignRange(range, false, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 50, 50, 1, 0, false, false, false);


	//Two bounds - exclusive, overlaping
	//Range: 50 (exclusive) - 50 (exclusive) asc
	//Expected result: Empty
	AssignString(endv, 50);
	AssignBound(endBound, true, endv);
	AssignString(startv, 50);
	AssignBound(startBound, false, startv);
	AssignRange(range, true, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 0, 0, 0, 0, false, false, false);

	//Range: 50 (exclusive) - 50 (exclusive) desc
	//Expected result: Empty
	AssignRange(range, false, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 0, 0, 0, 0, false, false, false);


	//Start after data - asc
	//Expected result: Empty
	AssignString(startv, 100);
	AssignBound(startBound, true, startv);
	AssignRange(range, true, 500, &startBound);
	TestRange<int64_t>(buf, range, 0, 0, 0, 0, false, true, false);

	//Start after data - desc
	//Expected result: Empty
	AssignString(startv, -2);
	AssignBound(startBound, true, startv);
	AssignRange(range, false, 500, &startBound);
	TestRange<int64_t>(buf, range, 0, 0, 0, 0, false, true, false);


	//Start at end
	//Range 98 (inclusive) - end asc
	//Expected result: 98
	AssignString(startv, 98);
	AssignBound(startBound, true, startv);
	AssignRange(range, true, 500, &startBound);
	TestRange<int64_t>(buf, range, 98, 98, 1, 0, false, true, false);

	//Range start - 98
	//Expected result: 98 desc
	AssignString(endv, 98);
	AssignBound(endBound, true, endv);
	AssignRange(range, false, 500, NULL, &endBound);
	TestRange<int64_t>(buf, range, 98, 98, 1, 0, true, false, false);


	//Range: 98 (exclusive) - end asc
	//Expected result: Empty
	AssignBound(startBound, false, startv);
	AssignRange(range, true, 500, &startBound);
	TestRange<int64_t>(buf, range, 0, 0, 0, 0, false, true, false);

	//Range start - 98 (exclusive) desc
	//Expected result: Empty
	AssignString(endv, 98);
	AssignBound(endBound, false, endv);
	AssignRange(range, false, 500, NULL, &endBound);
	TestRange<int64_t>(buf, range, 0, 0, 0, 0, true, false, false);


	//End before data - asc
	//Expected result: Empty
	AssignString(endv, -2);
	AssignBound(endBound, true, endv);
	AssignRange(range, true, 500, NULL, &endBound);
	TestRange<int64_t>(buf, range, 0, 0, 0, 0, true, false, false);

	//End before data - desc
	//Expected result: Empty
	AssignString(endv, 100);
	AssignBound(endBound, true, endv);
	AssignRange(range, false, 500, NULL, &endBound);
	TestRange<int64_t>(buf, range, 0, 0, 0, 0, true, false, false);


	//End at Start
	// Range: start - 0 (inclusive) asc
	//Expected result: 0
	AssignString(endv, 0);
	AssignBound(endBound, true, endv);
	AssignRange(range, true, 500, NULL, &endBound);
	TestRange<int64_t>(buf, range, 0, 0, 1, 0, true, false, false);

	// Range: start - 98 (inclusive) desc
	//Expected result: 98
	AssignString(endv, 98);
	AssignBound(endBound, true, endv);
	AssignRange(range, false, 500, NULL, &endBound);
	TestRange<int64_t>(buf, range, 98, 98, 1, 0, true, false, false);

	// Range: start - 0 (exclusive) asc
	//Expected result: Empty
	AssignString(endv, 0);
	AssignBound(endBound, false, endv);
	AssignRange(range, true, 500, NULL, &endBound);
	TestRange<int64_t>(buf, range, 0, 0, 0, 0, true, false, false);

	// Range: start - 98 (exclusive) desc
	//Expected result: Empty
	AssignString(endv, 98);
	AssignBound(endBound, false, endv);
	AssignRange(range, false, 500, NULL, &endBound);
	TestRange<int64_t>(buf, range, 0, 0, 0, 0, true, false, false);


	//Limited range
	//Range: start - end asc, limited to 5
	//Expected result: 0 - 8 (inclusive)
	AssignRange(range, true, 5);
	TestRange<int64_t>(buf, range, 0, 8, 5, 2, true, false, true);

	//Range: end - start desc, limited to 5
	//Expected result: 98-90 (inclusive)
	AssignRange(range, false, 5);
	TestRange<int64_t>(buf, range, 98, 90, 5, -2, true, false, true);

	//Start on ID
	//For a unique buffer there is one id per value, so a startID is essentially the same as using the exclusive flag.
	//Range: start - end asc, start on 0 ( 0 is excluded since it's assumed it's part of the last set)
	AssignString(startv, 0);
	AssignRange(range, true, 500, NULL, NULL, &startv);
	TestRange<int64_t>(buf, range, 2, 98, 49, 2, false, true, false);

	//Range: end - start desc, start on 98 ( 0 is excluded since it's assumed it's part of the last set)
	AssignString(endv, 98);	
	AssignRange(range, false, 500, NULL, NULL, &endv);
	TestRange<int64_t>(buf, range, 96, 0, 49, -2, false, true, false);

	//Combination - asc
	AssignString(endv, 94);
	AssignBound(endBound, false, endv);
	AssignString(startv, 80);
	AssignBound(startBound, false, startv);
	AssignRange(range, true, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 82, 92, 6, 2, false, false, false);

	AssignRange(range, true, 5, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 82, 90, 5, 2, false, false, true);

	//Combination - des
	AssignString(endv, 80);
	AssignBound(endBound, false, endv);
	AssignString(startv, 94);
	AssignBound(startBound, false, startv);
	AssignRange(range, false, 500, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 92, 82, 6, -2, false, false, false);

	AssignRange(range, false, 5, &startBound, &endBound);
	TestRange<int64_t>(buf, range, 92, 84, 5, -2, false, false, true);
}

void OneToManyRangeTest(IColumnBuffer* buf)
{
	ColumnWrites cw;
	std::vector<Cell> includes;

	//Insert 2 consecutive values from 0 - 98 (inclusive) in increments of 2 into buffer
	//e.g.  [
	//		(0 : [0 ,2]),
	//		(4 : [4, 6]),
	//		(8 : [8, 10]),
	//			etc...
	//		(96 : [96, 98])
	//		]

	for(int64_t i = 0; i < 100; i += 2){
		Cell inc;
		string rowId1;
		string rowId2;
		string value;
			
		AssignString(value, i);
			
		AssignString(rowId1, i);

		i += 2;

		AssignString(rowId2, i);

		inc.__set_value(value);
		inc.__set_rowID(rowId1);
		includes.push_back(inc);

		inc.__set_value(value);
		inc.__set_rowID(rowId2);
		includes.push_back(inc);
	}

	cw.__set_includes(includes);
	buf->Apply(cw);

	//should have 25 values, from 0-96 in increments of 4
	//Assert::AreEqual<int64_t >(buf->GetStatistic().total, 50);
	//Assert::AreEqual<int64_t >(buf->GetStatistic().unique, 25);
		
	string startv;
	string endv;
	RangeBound startBound;
	RangeBound endBound;
	RangeRequest range;
		
		
	//Entire Set
	//Range: Entire set ascending
	//Expected result: values 0 - 96 (inclusive) by 4s.
	AssignRange(range, true, 500);
	TestRangeMultiValue<int64_t>(buf, range, 0, 96, 25, 4, 2, 50, true, true, false);

	////Range: Entire set descending
	////Expected result: values 98 - 0 (inclusive) by -4s.
	//AssignRange(range, false, 500);
	//TestRangeMultiValue<int64_t>(buf, range, 98, 0, 25, -4, -2, 50, true, true, false);


	////Limit 5	- asc
	////Range: 0 (inclusive) - 16 (inclusive) ascending
	////Expected result: values 0 (inclusive) - 18 (inclusive)
	//AssignRange(range, true, 5);
	//TestRangeMultiValue<int64_t>(buf, range, 0, 16, 5, 4, 2, 10, true, true, false);

	////Limit 5 - desc
	////Range: 16 (inclusive) - 0 (inclusive) descending
	////Expected result: values 18 (inclusive) - 0 (inclusive)
	//AssignRange(range, false, 5);
	//TestRangeMultiValue<int64_t>(buf, range, 16, 0, 5, 4, 2, 10, true, true, false);


	//Start Exclusive - Non overlapping
	//Range: 0 (exclusive) - end (inclusive) ascending
	//Expected result: values 4 - 96 (inclusive)
	AssignString(startv, 0);
	AssignBound(startBound, false, startv);
	AssignRange(range, true, 500, &startBound);
	TestRangeMultiValue<int64_t>(buf, range, 4, 96, 24, 4, 2, 48, false, true, false);

	////Range: 0 (exclusive) - end (inclusive) descending
	////Expected result: values 96 - 4 (inclusive)
	//AssignString(startv, 0);
	//AssignBound(startBound, false, startv);
	//AssignRange(range, false, 500, &startBound);
	//TestRangeMultiValue<int64_t>(buf, range, 96, 4, 24, -4, -2, 48, false, true, false);


	//End Exclusive - Non overlapping
	//Range: begin (inclusive) - 96 (exclusive) ascending
	//Expected result: values 0 - 92 (inclusive)
	AssignString(endv, 96);
	AssignBound(endBound, false, endv);
	AssignRange(range, true, 500, NULL, &endBound);
	TestRangeMultiValue<int64_t>(buf, range, 0, 92, 24, 4, 2, 48, true, false, false);

	////Range: begin (inclusive) - 96 (exclusive) descending
	////Expected result: values 92 - 0 (inclusive)
	//AssignRange(range, false, 500, NULL, &endBound);
	//TestRangeMultiValue<int64_t>(buf, range, 96, 0, 24, -4, -2, 48, true, false, false);


	//Two bounds - inclusive, non overlapping
	//Range: 4 (inclusive) - 92 (inclusive) ascending
	//Expected result: values 4 - 92 (inclusive)
	AssignString(startv, 4);
	AssignBound(startBound, true, startv);
	AssignString(endv, 92);
	AssignBound(endBound, true, endv);
	AssignRange(range, true, 500, &startBound, &endBound);
	TestRangeMultiValue<int64_t>(buf, range, 4, 92, 23, 4, 2, 46, false, false, false);

	////Range: 2 (inclusive) - 96 (inclusive) descending
	////Expected result: values 96 - 2 (inclusive)
	//AssignRange(range, false, 500, &endBound, &startBound);
	//TestRangeMultiValue<int64_t>(buf, range, 92, 4, 23, -4, -2, 46, false, false, false);


	//Two bounds - exclusive, non overlapping
	//Range: 2 (exclusive) - 96 (exclusive) ascending
	//Expected result: values 4 - 92 (inclusive)
	AssignString(startv, 2);
	AssignBound(startBound, false, startv);		
	AssignString(endv, 96);
	AssignBound(endBound, false, endv);
	AssignRange(range, true, 500, &startBound, &endBound);
	TestRangeMultiValue<int64_t>(buf, range, 4, 92, 23, 4, 2, 46, false, false, false);

	////Range: 2 (exclusive) - 96 (exclusive) descending
	////Expected result: values 94 - 4 (inclusive)
	//AssignRange(range, false, 500, &endBound, &startBound);
	//TestRangeMultiValue<int64_t>(buf, range, 92, 4, 23, -4, -2, 46, false, false, false);


	////Two bounds - inclusive, overlapping
	////Range: 50 (inclusive) - 50 (inclusive) ascending
	////Expected result: 50
	//AssignString(startv, 50);
	//AssignBound(startBound, true, startv);
	//AssignString(endv, 50);
	//AssignBound(endBound, true, endv);
	//AssignRange(range, true, 500, &startBound, &endBound);
	//TestRangeMultiValue<int64_t>(buf, range, 50, 50, 1, 0, 0, 1, false, false, false);

	////Range: 50 (inclusive) - 50 (inclusive) desc
	////Expected result: 50
	//AssignRange(range, false, 500, &startBound, &endBound);
	//TestRangeMultiValue<int64_t>(buf, range, 50, 50, 1, 0, 0, 1, false, false, false);


	//Two bounds - exclusive, overlaping
	//Range: 50 (exclusive) - 50 (exclusive) asc
	//Expected result: Empty
	AssignString(endv, 50);
	AssignBound(endBound, true, endv);
	AssignString(startv, 50);
	AssignBound(startBound, false, startv);
	AssignRange(range, true, 500, &startBound, &endBound);
	TestRangeMultiValue<int64_t>(buf, range, 0, 0, 0, 0, 0, 0, false, false, false);

	//Range: 50 (exclusive) - 50 (exclusive) desc
	//Expected result: Empty
	AssignRange(range, false, 500, &startBound, &endBound);
	TestRangeMultiValue<int64_t>(buf, range, 0, 0, 0, 0, 0, 0, false, false, false);


	//Start after data - asc
	//Expected result: Empty
	AssignString(startv, 100);
	AssignBound(startBound, true, startv);
	AssignRange(range, true, 500, &startBound);
	TestRangeMultiValue<int64_t>(buf, range, 0, 0, 0, 0, 0, 0, false, true, false);

	////Start after data - desc
	////Expected result: Empty
	//AssignRange(range, false, 500, &startBound);
	//TestRangeMultiValue<int64_t>(buf, range, 0, 0, 0, 0, 0, 0, false, true, false);


	////Start at end
	////Range 98 (inclusive) - end asc
	////Expected result: 98
	//AssignString(startv, 98);
	//AssignBound(startBound, true, startv);
	//AssignRange(range, true, 500, &startBound);
	//TestRangeMultiValue<int64_t>(buf, range, 98, 98, 1, 0, 0, 1, false, true, false);

	////Range 98 (inclusive) - end asc
	////Expected result: 98
	//AssignRange(range, false, 500, &startBound);
	//TestRangeMultiValue<int64_t>(buf, range, 98, 98, 1, 0, 0, 1, false, true, false);


	//Range: 98 (exclusive) - end asc
	//Expected result: Empty
	AssignString(startv, 98);
	AssignBound(startBound, false, startv);
	AssignRange(range, true, 500, &startBound);
	TestRangeMultiValue<int64_t>(buf, range, 0, 0, 0, 0, 0, 0, false, true, false);

	////Range 98 (exclusive) - end desc
	////Expected result: Empty
	//AssignRange(range, false, 500, &startBound);
	//TestRangeMultiValue<int64_t>(buf, range, 0, 0, 0, 0, 0, 0, false, true, false);


	//End before data - asc
	//Expected result: Empty
	AssignString(endv, -2);
	AssignBound(endBound, true, endv);
	AssignRange(range, true, 500, NULL, &endBound);
	TestRangeMultiValue<int64_t>(buf, range, 0, 0, 0, 0, 0, 0, true, false, false);

	////End before data - desc
	////Expected result: Empty
	//AssignRange(range, false, 500, NULL, &endBound);
	//TestRangeMultiValue<int64_t>(buf, range, 0, 0, 0, 0, 0, 0, true, false, false);


	////End at Start
	//// Range: start - 0 (inclusive) asc
	////Expected result: 0
	//AssignString(endv, 0);
	//AssignBound(endBound, true, endv);
	//AssignRange(range, true, 500, NULL, &endBound);
	//TestRangeMultiValue<int64_t>(buf, range, 0, 0, 1, 0, 0, 1, true, false, false);

	//// Range: start - 0 (inclusive) desc
	////Expected result: 0
	//AssignRange(range, false, 500, NULL, &endBound);
	//TestRangeMultiValue<int64_t>(buf, range, 0, 0, 1, 0, 0, 1, true, false, false);


	// Range: start - 0 (exclusive) asc
	//Expected result: Empty
	AssignString(endv, 0);
	AssignBound(endBound, false, endv);
	AssignRange(range, true, 500, NULL, &endBound);
	TestRangeMultiValue<int64_t>(buf, range, 0, 0, 0, 0, 0, 0, true, false, false);

	//// Range: start - 0 (exclusive) desc
	////Expected result: Empty
	//AssignRange(range, false, 500, NULL, &endBound);
	//TestRangeMultiValue<int64_t>(buf, range, 0, 0, 0, 0, 0, 0, true, false, false);


	////Limited range
	////Range: start - end asc, limited to 5
	////Expected result: 0 - 16 (inclusive)
	//AssignRange(range, true, 5);
	//TestRangeMultiValue<int64_t>(buf, range, 0, 16, 5, 4, 2, 10, true, false, true);

	////Range: end - start desc, limited to 5
	////Expected result: 98-80 (inclusive)
	//AssignRange(range, false, 5);
	//TestRangeMultiValue<int64_t>(buf, range, 98, 80, 5, -4, -2, 10, false, true, true);


	////Start on ID  - 0
	////Range: start - end asc, start on 0
	//AssignString(startv, 0);
	//AssignBound(startBound, true, startv);
	//AssignRange(range, true, 500, &startBound, NULL, &startv);
	//TestRangeMultiValue<int64_t>(buf, range, 0, 98, 25, 4, 2, 50, false, true, false);

	////Range: end - start desc, start on 98
	//AssignString(endv, 98);
	//AssignBound(endBound, true, endv);
	//AssignRange(range, false, 500, NULL, &endBound, &endv);
	//TestRangeMultiValue<int64_t>(buf, range, 98, 0, 25, -4, -2, 50, true, false, false);


	////Start on ID  - 2
	////Range: start - end asc, start on 2
	//AssignString(startv, 2);
	//AssignBound(startBound, true, startv);
	//AssignRange(range, true, 500, &startBound, NULL, &startv);
	//TestRangeMultiValue<int64_t>(buf, range, 4, 98, 24, 4, 2, 48, false, true, false);

	////Range: end - start desc, start on 98 (0 is excluded since it's assumed it's part of the last set)
	//AssignString(endv, 98);
	//AssignBound(endBound, true, endv);
	//AssignRange(range, false, 500, NULL, &endBound, &endv);
	//TestRangeMultiValue<int64_t>(buf, range, 98, 4, 24, -4, -2, 48, true, false, false);


	////Combination
	////Range: 80 (exclusive) - 94 (exclusive)  ascending	limit 500
	////Expected result: 84-90 (inclusive)
	//AssignString(startv, 80);
	//AssignBound(startBound, false, startv);
	//AssignString(endv, 94);
	//AssignBound(endBound, false, endv);
	//AssignRange(range, true, 500, &startBound, &endBound);
	//TestRangeMultiValue<int64_t>(buf, range, 84, 88, 2, 4, 2, 4, false, false, false);

	////Range: 80 (exclusive) - 94 (exclusive)  ascending	limit 5
	////Expected result: 84-90 (inclusive)
	//AssignRange(range, true, 5, &startBound, &endBound);
	//TestRangeMultiValue<int64_t>(buf, range, 84, 88, 2, 4, 2, 4, false, false, true);
}