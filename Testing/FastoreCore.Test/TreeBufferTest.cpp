#include "stdafx.h"
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <sstream>
#include <iostream>
#include <hash_set>
#include <algorithm>

#include "Schema\standardtypes.h"
#include "Column\TreeBuffer.h"

using namespace std;

template <class T>
inline std::string to_string(const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();

}

TEST_CLASS(TreeBufferTest)
{
public:
	
	TEST_METHOD(TreeBufferRangeTests)
	{
	
		//--- THIS SUBSET OF BEHAVIOR SHOULD BE IDENTICAL TO UNIQUE BUFFER ---

		///TODO: Update limited behavior to reflect BoF/EoF semantics.
		//Unique buffer -- one key has one and only one value
		TreeBuffer buf(standardtypes::Int, standardtypes::Int);

		ColumnWrites cw;
		std:vector<Include> includes;

		//Insert values 0 - 98 (inclusive) in increments of 2 into buffer
		for (int i = 0; i < 100; i += 2)
		{
			Include inc;
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
		buf.Apply(cw);

		Assert::AreEqual<long long>(buf.GetStatistic().total, 50);
		Assert::AreEqual<long long>(buf.GetStatistic().unique, 50);

		string startv;
		string endv;
		RangeBound startBound;
		RangeBound endBound;
		RangeRequest range;

		//Entire Set
		//Range: Entire set ascending
		//Expected result: values 0 - 98 (inclusive) by 2s.
		AssignRange(range, true, 500);
		TestRange(buf, range, 0, 98, 50, 2, true, true, false);

		//Range: Entire set descending
		//Expected result: values 98 - 0 (inclusive) by -2s.
		AssignRange(range, false, 500);
		TestRange(buf, range, 98, 0, 50, -2, true, true, false);


		//Start Exclusive - Non overlapping
		//Range: 0 (exclusive) - end (inclusive) 
		//Expected result: values 2 - 98 (inclusive)
		AssignString(startv, 0);
		AssignBound(startBound, false, startv);
		AssignRange(range, true, 500, &startBound);
		TestRange(buf, range, 2, 98, 49, 2, false, true, false);

		//Range:  98 (inc) - 0 (exclusive) 
		//Expected result: values 98 - 2 (inclusive)
		AssignString(endv, 0);
		AssignBound(endBound, false, startv);
		AssignRange(range, false, 500, NULL, &endBound);
		TestRange(buf, range, 98, 2, 49, -2, true, false, false);


		//End Exclusive - Non overlapping
		//Range: begin (inclusive) - 98 (exclusive)
		//Expected result: values 0 - 96 (inclusive)
		AssignString(endv, 98);
		AssignBound(endBound, false, endv);
		AssignRange(range, true, 500, NULL, &endBound);
		TestRange(buf, range, 0, 96, 49, 2, true, false, false);

		//Range: 98 (exclusive)  -  0 (inclusive)
		//Expected result: values 96 - 0 (inclusive)
		AssignString(startv, 98);
		AssignBound(startBound, false, startv);
		AssignRange(range, false, 500, &startBound);
		TestRange(buf, range, 96, 0, 49, -2, false, true, false);


		//Two bounds - inclusive, non overlapping
		//Range: 2 (inclusive) - 96 (inclusive)
		//Expected result: values 2 - 96 (inclusive)
		AssignString(endv, 96);
		AssignBound(endBound, true, endv);
		AssignString(startv, 2);
		AssignBound(startBound, true, startv);		
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange(buf, range, 2, 96, 48, 2, false, false, false);

		//Range: 96 (inclusive) - 2 (inclusive)
		//Expected result: values 96 - 2 (inclusive)
		AssignString(endv, 2);
		AssignBound(endBound, true, endv);
		AssignString(startv, 96);
		AssignBound(startBound, true, startv);		
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange(buf, range, 96, 2, 48, -2, false, false, false);

		//Range: 3 (inclusive) - 95 (inclusive)
		//Expected result: values 4 - 94 (inclusive)
		AssignString(endv, 95);
		AssignBound(endBound, true, endv);
		AssignString(startv, 3);
		AssignBound(startBound, true, startv);		
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange(buf, range, 4, 94, 46, 2, false, false, false);

		//Range: 95 (inclusive) - 3 (inclusive)
		//Expected result: values 94 - 4 (inclusive)
		AssignString(endv, 3);
		AssignBound(endBound, true, endv);
		AssignString(startv, 95);
		AssignBound(startBound, true, startv);		
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange(buf, range, 94, 4, 46, -2, false, false, false);


		//Two bounds - exclusive, non overlapping
		//Range: 2 (exclusive) - 96 (exclusive)
		//Expected result: values 4 - 94 (inclusive)
		AssignString(endv, 96);
		AssignBound(endBound, false, endv);
		AssignString(startv, 2);
		AssignBound(startBound, false, startv);		
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange(buf, range, 4, 94, 46, 2, false, false, false);

		//Range: 96 (exclusive) - 2 (exclusive)
		//Expected result: values 94 - 4 (inclusive)
		AssignString(endv, 2);
		AssignBound(endBound, false, endv);
		AssignString(startv, 96);
		AssignBound(startBound, false, startv);		
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange(buf, range, 94, 4, 46, -2, false, false, false);

		//Range: 3 (exclusive) - 95 (exclusive)
		//Expected result: values 4 - 94 (exclusive)
		AssignString(endv, 95);
		AssignBound(endBound, false, endv);
		AssignString(startv, 3);
		AssignBound(startBound, false, startv);		
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange(buf, range, 4, 94, 46, 2, false, false, false);

		//Range: 95 (exclusive) - 3 (exclusive)
		//Expected result: values 94 - 4 (inclusive)
		AssignString(endv, 3);
		AssignBound(endBound, false, endv);
		AssignString(startv, 95);
		AssignBound(startBound, false, startv);		
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange(buf, range, 94, 4, 46, -2, false, false, false);


		//Two bounds - inclusive, overlapping
		//Range: 50 (inclusive) - 50 (inclusive) ascending
		//Expected result: 50
		AssignString(endv, 50);
		AssignBound(endBound, true, endv);
		AssignString(startv, 50);
		AssignBound(startBound, true, startv);
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange(buf, range, 50, 50, 1, 0, false, false, false);

		//Range: 50 (inclusive) - 50 (inclusive) desc
		//Expected result: 50
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange(buf, range, 50, 50, 1, 0, false, false, false);


		//Two bounds - exclusive, overlaping
		//Range: 50 (exclusive) - 50 (exclusive) asc
		//Expected result: Empty
		AssignString(endv, 50);
		AssignBound(endBound, true, endv);
		AssignString(startv, 50);
		AssignBound(startBound, false, startv);
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange(buf, range, 0, 0, 0, 0, false, false, false);

		//Range: 50 (exclusive) - 50 (exclusive) desc
		//Expected result: Empty
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange(buf, range, 0, 0, 0, 0, false, false, false);


		//Start after data - asc
		//Expected result: Empty
		AssignString(startv, 100);
		AssignBound(startBound, true, startv);
		AssignRange(range, true, 500, &startBound);
		TestRange(buf, range, 0, 0, 0, 0, false, true, false);

		//Start after data - desc
		//Expected result: Empty
		AssignString(startv, -2);
		AssignBound(startBound, true, startv);
		AssignRange(range, false, 500, &startBound);
		TestRange(buf, range, 0, 0, 0, 0, false, true, false);


		//Start at end
		//Range 98 (inclusive) - end asc
		//Expected result: 98
		AssignString(startv, 98);
		AssignBound(startBound, true, startv);
		AssignRange(range, true, 500, &startBound);
		TestRange(buf, range, 98, 98, 1, 0, false, true, false);

		//Range start - 98
		//Expected result: 98 desc
		AssignString(endv, 98);
		AssignBound(endBound, true, endv);
		AssignRange(range, false, 500, NULL, &endBound);
		TestRange(buf, range, 98, 98, 1, 0, true, false, false);


		//Range: 98 (exclusive) - end asc
		//Expected result: Empty
		AssignBound(startBound, false, startv);
		AssignRange(range, true, 500, &startBound);
		TestRange(buf, range, 0, 0, 0, 0, false, true, false);

		//Range start - 98 (exclusive) desc
		//Expected result: Empty
		AssignString(endv, 98);
		AssignBound(endBound, false, endv);
		AssignRange(range, false, 500, NULL, &endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);


		//End before data - asc
		//Expected result: Empty
		AssignString(endv, -2);
		AssignBound(endBound, true, endv);
		AssignRange(range, true, 500, NULL, &endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);

		//End before data - desc
		//Expected result: Empty
		AssignString(endv, 100);
		AssignBound(endBound, true, endv);
		AssignRange(range, false, 500, NULL, &endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);


		//End at Start
		// Range: start - 0 (inclusive) asc
		//Expected result: 0
		AssignString(endv, 0);
		AssignBound(endBound, true, endv);
		AssignRange(range, true, 500, NULL, &endBound);
		TestRange(buf, range, 0, 0, 1, 0, true, false, false);

		// Range: start - 98 (inclusive) desc
		//Expected result: 98
		AssignString(endv, 98);
		AssignBound(endBound, true, endv);
		AssignRange(range, false, 500, NULL, &endBound);
		TestRange(buf, range, 98, 98, 1, 0, true, false, false);

		// Range: start - 0 (exclusive) asc
		//Expected result: Empty
		AssignString(endv, 0);
		AssignBound(endBound, false, endv);
		AssignRange(range, true, 500, NULL, &endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);

		// Range: start - 98 (exclusive) desc
		//Expected result: Empty
		AssignString(endv, 98);
		AssignBound(endBound, false, endv);
		AssignRange(range, false, 500, NULL, &endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);


		//Limited range
		//Range: start - end asc, limited to 5
		//Expected result: 0 - 8 (inclusive)
		AssignRange(range, true, 5);
		TestRange(buf, range, 0, 8, 5, 2, true, false, true);

		//Range: end - start desc, limited to 5
		//Expected result: 98-90 (inclusive)
		AssignRange(range, false, 5);
		TestRange(buf, range, 98, 90, 5, -2, true, false, true);

		//Start on ID
		//For a unique buffer there is one id per value, so a startID is essentially the same as using the exclusive flag.
		//Range: start - end asc, start on 0 ( 0 is excluded since it's assumed it's part of the last set)
		AssignString(startv, 0);
		AssignRange(range, true, 500, NULL, NULL, &startv);
		TestRange(buf, range, 2, 98, 49, 2, false, true, false);

		//Range: end - start desc, start on 98 ( 0 is excluded since it's assumed it's part of the last set)
		AssignString(endv, 98);	
		AssignRange(range, false, 500, NULL, NULL, &endv);
		TestRange(buf, range, 96, 0, 49, -2, false, true, false);

		//Combination - asc
		AssignString(endv, 94);
		AssignBound(endBound, false, endv);
		AssignString(startv, 80);
		AssignBound(startBound, false, startv);
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange(buf, range, 82, 92, 6, 2, false, false, false);

		AssignRange(range, true, 5, &startBound, &endBound);
		TestRange(buf, range, 82, 90, 5, 2, false, false, true);

		//Combination - des
		AssignString(endv, 80);
		AssignBound(endBound, false, endv);
		AssignString(startv, 94);
		AssignBound(startBound, false, startv);
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange(buf, range, 92, 82, 6, -2, false, false, false);

		AssignRange(range, false, 5, &startBound, &endBound);
		TestRange(buf, range, 92, 84, 5, -2, false, false, true);

	 }

	 //--- END UNIQUE BUFFER COPY --------------------------------

	TEST_METHOD(RangeMultiRowTests)
	{										 
		TreeBuffer buf(standardtypes::Int, standardtypes::Int);

		ColumnWrites cw;
		std:vector<Include> includes;

		//Insert 2 consecutive values from 0 - 98 (inclusive) in increments of 2 into buffer
		//e.g.  [
		//		(0 : [0 ,2]),
		//		(4 : [4, 6]),
		//		(8 : [8, 10]),
		//			etc...
		//		(96 : [96, 98])
		//		]

		for(int i = 0; i < 100; i += 2){
			Include inc;
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
		buf.Apply(cw);

		//should have 25 values, from 0-96 in increments of 4
		Assert::AreEqual<long long>(buf.GetStatistic().total, 50);
		Assert::AreEqual<long long>(buf.GetStatistic().unique, 25);
		
		string startv;
		string endv;
		RangeBound startBound;
		RangeBound endBound;
		RangeRequest range;
		
		
		//Entire Set
		//Range: Entire set ascending
		//Expected result: values 0 - 96 (inclusive) by 4s.
		AssignRange(range, true, 500);
		TestRangeMultiValue(buf, range, 0, 96, 25, 4, 2, 50, true, true, false);

		////Range: Entire set descending
		////Expected result: values 98 - 0 (inclusive) by -4s.
		//AssignRange(range, false, 500);
		//TestRangeMultiValue(buf, range, 98, 0, 25, -4, -2, 50, true, true, false);

		//Start Exclusive - Non overlapping
		//Range: 0 (exclusive) - end (inclusive) ascending
		//Expected result: values 4 - 96 (inclusive)
		AssignString(startv, 0);
		AssignBound(startBound, false, startv);
		AssignRange(range, true, 500, &startBound);
		TestRangeMultiValue(buf, range, 4, 96, 24, 4, 2, 48, false, true, false);

		////Range: 0 (exclusive) - end (inclusive) descending
		////Expected result: values 96 - 4 (inclusive)
		//AssignString(startv, 0);
		//AssignBound(startBound, false, startv);
		//AssignRange(range, false, 500, &startBound);
		//TestRangeMultiValue(buf, range, 96, 4, 24, -4, -2, 48, false, true, false);


		//End Exclusive - Non overlapping
		//Range: begin (inclusive) - 96 (exclusive) ascending
		//Expected result: values 0 - 92 (inclusive)
		AssignString(endv, 96);
		AssignBound(endBound, false, endv);
		AssignRange(range, true, 500, NULL, &endBound);
		TestRangeMultiValue(buf, range, 0, 92, 24, 4, 2, 48, true, false, false);

		////Range: begin (inclusive) - 96 (exclusive) descending
		////Expected result: values 92 - 0 (inclusive)
		//AssignRange(range, false, 500, NULL, &endBound);
		//TestRangeMultiValue(buf, range, 96, 0, 24, -4, -2, 48, true, false, false);


		//Two bounds - inclusive, non overlapping
		//Range: 4 (inclusive) - 92 (inclusive) ascending
		//Expected result: values 4 - 92 (inclusive)
		AssignString(startv, 4);
		AssignBound(startBound, true, startv);
		AssignString(endv, 92);
		AssignBound(endBound, true, endv);
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRangeMultiValue(buf, range, 4, 92, 23, 4, 2, 46, false, false, false);

		////Range: 2 (inclusive) - 96 (inclusive) descending
		////Expected result: values 96 - 2 (inclusive)
		//AssignRange(range, false, 500, &endBound, &startBound);
		//TestRangeMultiValue(buf, range, 92, 4, 23, -4, -2, 46, false, false, false);


		//Two bounds - exclusive, non overlapping
		//Range: 2 (exclusive) - 96 (exclusive) ascending
		//Expected result: values 4 - 92 (inclusive)
		AssignString(startv, 2);
		AssignBound(startBound, false, startv);		
		AssignString(endv, 96);
		AssignBound(endBound, false, endv);
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRangeMultiValue(buf, range, 4, 92, 23, 4, 2, 46, false, false, false);

		////Range: 2 (exclusive) - 96 (exclusive) descending
		////Expected result: values 94 - 4 (inclusive)
		//AssignRange(range, false, 500, &endBound, &startBound);
		//TestRangeMultiValue(buf, range, 92, 4, 23, -4, -2, 46, false, false, false);


		////Two bounds - inclusive, overlapping
		////Range: 50 (inclusive) - 50 (inclusive) ascending
		////Expected result: 50
		//AssignString(startv, 50);
		//AssignBound(startBound, true, startv);
		//AssignString(endv, 50);
		//AssignBound(endBound, true, endv);
		//AssignRange(range, true, 500, &startBound, &endBound);
		//TestRangeMultiValue(buf, range, 50, 50, 1, 0, 0, 1, false, false, false);

		////Range: 50 (inclusive) - 50 (inclusive) desc
		////Expected result: 50
		//AssignRange(range, false, 500, &startBound, &endBound);
		//TestRangeMultiValue(buf, range, 50, 50, 1, 0, 0, 1, false, false, false);


		//Two bounds - exclusive, overlaping
		//Range: 50 (exclusive) - 50 (exclusive) asc
		//Expected result: Empty
		AssignString(endv, 50);
		AssignBound(endBound, true, endv);
		AssignString(startv, 50);
		AssignBound(startBound, false, startv);
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRangeMultiValue(buf, range, 0, 0, 0, 0, 0, 0, false, false, false);

		//Range: 50 (exclusive) - 50 (exclusive) desc
		//Expected result: Empty
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRangeMultiValue(buf, range, 0, 0, 0, 0, 0, 0, false, false, false);


		//Start after data - asc
		//Expected result: Empty
		AssignString(startv, 100);
		AssignBound(startBound, true, startv);
		AssignRange(range, true, 500, &startBound);
		TestRangeMultiValue(buf, range, 0, 0, 0, 0, 0, 0, false, true, false);

		////Start after data - desc
		////Expected result: Empty
		//AssignRange(range, false, 500, &startBound);
		//TestRangeMultiValue(buf, range, 0, 0, 0, 0, 0, 0, false, true, false);


		////Start at end
		////Range 98 (inclusive) - end asc
		////Expected result: 98
		//AssignString(startv, 98);
		//AssignBound(startBound, true, startv);
		//AssignRange(range, true, 500, &startBound);
		//TestRangeMultiValue(buf, range, 98, 98, 1, 0, 0, 1, false, true, false);

		////Range 98 (inclusive) - end asc
		////Expected result: 98
		//AssignRange(range, false, 500, &startBound);
		//TestRangeMultiValue(buf, range, 98, 98, 1, 0, 0, 1, false, true, false);

		//Range: 98 (exclusive) - end asc
		//Expected result: Empty
		AssignString(startv, 98);
		AssignBound(startBound, false, startv);
		AssignRange(range, true, 500, &startBound);
		TestRangeMultiValue(buf, range, 0, 0, 0, 0, 0, 0, false, true, false);

		////Range 98 (exclusive) - end desc
		////Expected result: Empty
		//AssignRange(range, false, 500, &startBound);
		//TestRangeMultiValue(buf, range, 0, 0, 0, 0, 0, 0, false, true, false);


		//End before data - asc
		//Expected result: Empty
		AssignString(endv, -2);
		AssignBound(endBound, true, endv);
		AssignRange(range, true, 500, NULL, &endBound);
		TestRangeMultiValue(buf, range, 0, 0, 0, 0, 0, 0, true, false, false);

		////End before data - desc
		////Expected result: Empty
		//AssignRange(range, false, 500, NULL, &endBound);
		//TestRangeMultiValue(buf, range, 0, 0, 0, 0, 0, 0, true, false, false);


		////End at Start
		//// Range: start - 0 (inclusive) asc
		////Expected result: 0
		//AssignString(endv, 0);
		//AssignBound(endBound, true, endv);
		//AssignRange(range, true, 500, NULL, &endBound);
		//TestRangeMultiValue(buf, range, 0, 0, 1, 0, 0, 1, true, false, false);

		//// Range: start - 0 (inclusive) desc
		////Expected result: 0
		//AssignRange(range, false, 500, NULL, &endBound);
		//TestRangeMultiValue(buf, range, 0, 0, 1, 0, 0, 1, true, false, false);

		// Range: start - 0 (exclusive) asc
		//Expected result: Empty
		AssignString(endv, 0);
		AssignBound(endBound, false, endv);
		AssignRange(range, true, 500, NULL, &endBound);
		TestRangeMultiValue(buf, range, 0, 0, 0, 0, 0, 0, true, false, false);

		//// Range: start - 0 (exclusive) desc
		////Expected result: Empty
		//AssignRange(range, false, 500, NULL, &endBound);
		//TestRangeMultiValue(buf, range, 0, 0, 0, 0, 0, 0, true, false, false);


		////Limited range
		////Range: start - end asc, limited to 5
		////Expected result: 0 - 16 (inclusive)
		//AssignRange(range, true, 5);
		//TestRangeMultiValue(buf, range, 0, 16, 5, 4, 2, 10, true, false, true);

		////Range: end - start desc, limited to 5
		////Expected result: 98-80 (inclusive)
		//AssignRange(range, false, 5);
		//TestRangeMultiValue(buf, range, 98, 80, 5, -4, -2, 10, false, true, true);

		////Start on ID
		////For a unique buffer there is one id per value, so a startID is essentially the same as using the exclusive flag.
		////Range: start - end asc, start on 0 (0 is excluded since it's assumed it's part of the last set)
		//AssignString(startv, 0);
		//AssignBound(startBound, true, startv);
		//AssignRange(range, true, 500, &startBound, NULL, &startv);
		//TestRangeMultiValue(buf, range, 4, 98, 24, 4, 2, 48, false, true, false);

		////Range: end - start desc, start on 98 (0 is excluded since it's assumed it's part of the last set)
		//AssignString(endv, 98);
		//AssignBound(endBound, true, endv);
		//AssignRange(range, false, 500, NULL, &endBound, &endv);
		//TestRangeMultiValue(buf, range, 98, 4, 24, -4, -2, 48, true, false, false);


		////Combination
		////Range: 80 (exclusive) - 94 (exclusive)  ascending	limit 500
		////Expected result: 84-90 (inclusive)
		//AssignString(startv, 80);
		//AssignBound(startBound, false, startv);
		//AssignString(endv, 94);
		//AssignBound(endBound, false, endv);
		//AssignRange(range, true, 500, &startBound, &endBound);
		//TestRangeMultiValue(buf, range, 84, 88, 2, 4, 2, 4, false, false, false);

		////Combination
		////Range: 80 (exclusive) - 94 (exclusive)  ascending	limit 5
		////Expected result: 84-90 (inclusive)
		//AssignRange(range, true, 5, &startBound, &endBound);
		//TestRangeMultiValue(buf, range, 84, 88, 2, 4, 2, 4, false, false, true);
	}

	void AssignString(string& str, int value)
	{
		str.assign((const char*)&value, sizeof(int));
	}

	void AssignBound(RangeBound& bound, bool inclusive, string& value)
	{
		bound = RangeBound();
		bound.__set_inclusive(inclusive);
		bound.__set_value(value);
	}

	void AssignRange(RangeRequest& range, bool ascending, int limit, RangeBound* start = NULL, RangeBound* end = NULL, string* rowId = NULL)
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

	void TestRange(TreeBuffer& buf, RangeRequest range, int expectedStart, int expectedEnd, int expectedValuesCount, int increment, bool expectBOF, bool expectEOF, bool expectLimited)
	{
		RangeResult result = buf.GetRows(range);
		
		//Right number of values...
		Assert::AreEqual<int>(result.valueRowsList.size(), expectedValuesCount);

		int expectedNum = expectedStart;
		for (int i = 0; i < result.valueRowsList.size(); i++)
		{
			//Item is a value-rows  A - 1, 2, 3
			auto item = result.valueRowsList[i];

			int value = *(int*)item.value.data();

			//Value should be our expected number;
			Assert::AreEqual<int>(expectedNum, value);

			//should be one id per value (unique buffer)
			Assert::IsTrue(item.rowIDs.size() == 1);

			//id should be expected number
			Assert::AreEqual<int>(expectedNum, *(int*)item.rowIDs[0].data());

			expectedNum += increment;
		}

		//We should see the expectedEnd + increment if we've iterated all values, or just expected end if we didn't iterate.
		Assert::AreEqual<int>(expectedNum, expectedEnd + (result.valueRowsList.size() > 0 ? increment : 0));
		Assert::IsTrue(result.bof == expectBOF);
		Assert::IsTrue(result.eof == expectEOF);
		Assert::IsTrue(result.limited == expectLimited);
	}

	void TestRangeMultiValue(TreeBuffer& buf, RangeRequest range, int expectedValueStart, int expectedValueEnd, int expectedValueCount, int valueIncrement, int rowIdIncrement, int expectedTotalRowIds, bool expectBOF, bool expectEOF, bool expectLimited)
	{
		RangeResult result = buf.GetRows(range);

		int expectedNum = expectedValueStart;
		int expectedTotal = 0;
		for (int i = 0; i < result.valueRowsList.size(); i++)
		{
			//Item is a value-rows  A - 1, 2, 3
			auto item = result.valueRowsList[i];

			int value = *(int*)item.value.data();

			//Value should be our expected number;
			Assert::AreEqual<int>(expectedNum, value);

			for(int j = 0; j < item.rowIDs.size(); j++)
			{
				//each id should be expected number
				Assert::AreEqual<int>(expectedNum, *(int*)item.rowIDs[j].data());

				expectedNum += rowIdIncrement;
				expectedTotal++;
			}
			
		}

		//We should see the expectedEnd + increment if we've iterated all values, or just expected end if we didn't iterate.
		Assert::AreEqual<int>(expectedNum, expectedValueEnd + (result.valueRowsList.size() > 0 ? valueIncrement : 0));	
		//Right number of values
		Assert::AreEqual<int>(result.valueRowsList.size(), expectedValueCount);
		//Right number of total values & rowIds
		Assert::AreEqual<int>(expectedTotal, expectedTotalRowIds);

		Assert::IsTrue(result.bof == expectBOF);
		Assert::IsTrue(result.eof == expectEOF);
		Assert::IsTrue(result.limited == expectLimited);
	}

	TEST_METHOD(TreeBufferIncludeExclude)
	{
		throw "Not yet implemented";
		//TreeBuffer buf(standardtypes::Int, standardtypes::Int);

		//bool result;
		//int v = 0;
		//int k = 0;

		////Non existing inserts should be ok
		//result = buf.Include(&v, &k);
		//Assert::AreEqual(result == true);

		////Duplicate insertions should not insert
		//result = buf.Include(&v, &k);
		//Assert::AreEqual(result == false);

		////Duplicate keys should not insert
		//v = 2;
	 //   k = 0;
		//result = buf.Include(&v, &k);
		//Assert::AreEqual(result == false);

		////End of this should still be zero
		//k = 0;
		//void* value = buf.GetValue(&k);
		//Assert::AreEqual(*(int*)value == 0);

		////Values not present should not exclude
		//v = 1;
		//k = 0;
		//result = buf.Exclude(&v, &k);
		//Assert::AreEqual(result == false);

		////Keys not present should not exclude
		//v = 0;
		//k = 1;
		//result = buf.Exclude(&k);
		//Assert::AreEqual(result == false);

		//result = buf.Exclude(&v, &k);
		//Assert::AreEqual(result == false);

		////Keys present should exclude
		//v = 0;
		//k = 0;
		//result = buf.Exclude(&v, &k);
		//Assert::AreEqual(result == true);

		////A bunch of insertions should work...
		//int numrows = 10000;
		//for (int i = 0; i <= numrows; i++)
		//{
		//	result = buf.Include(&i,&i);
		//	Assert::AreEqual(result == true);
		//}

		////A bunch of exclusions should work...
		//for (int i = numrows / 2; i <= numrows; i++)
		//{
		//	result = buf.Exclude(&i,&i);
		//	Assert::AreEqual(result == true);
		//}

		////All the values should still be the same...
		//for (int i = 0; i < numrows / 2; i++)
		//{
		//	value = buf.GetValue(&i);
		//	Assert::AreEqual(*(int*)value == i);
		//}
	}
};