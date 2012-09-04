#include "stdafx.h"
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "Column\IdentityBuffer.h"
#include "Extensions.h"

using namespace std;


TEST_CLASS(IdentityBufferTest)
{
public:
	
	TEST_METHOD(IdentityRangeTests)
	{
		//Identity buffer -- one key has one and only one value. Value is the same type and 'number' as key.
		IdentityBuffer buf(standardtypes::Int);

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
		TestRange<int>(buf, range, 0, 98, 50, 2, true, true, false);

		//Range: Entire set descending
		//Expected result: values 98 - 0 (inclusive) by -2s.
		AssignRange(range, false, 500);
		TestRange<int>(buf, range, 98, 0, 50, -2, true, true, false);


		//Start Exclusive - Non overlapping
		//Range: 0 (exclusive) - end (inclusive) 
		//Expected result: values 2 - 98 (inclusive)
		AssignString(startv, 0);
		AssignBound(startBound, false, startv);
		AssignRange(range, true, 500, &startBound);
		TestRange<int>(buf, range, 2, 98, 49, 2, false, true, false);

		//Range:  98 (inc) - 0 (exclusive) 
		//Expected result: values 98 - 2 (inclusive)
		AssignString(endv, 0);
		AssignBound(endBound, false, startv);
		AssignRange(range, false, 500, NULL, &endBound);
		TestRange<int>(buf, range, 98, 2, 49, -2, true, false, false);


		//End Exclusive - Non overlapping
		//Range: begin (inclusive) - 98 (exclusive)
		//Expected result: values 0 - 96 (inclusive)
		AssignString(endv, 98);
		AssignBound(endBound, false, endv);
		AssignRange(range, true, 500, NULL, &endBound);
		TestRange<int>(buf, range, 0, 96, 49, 2, true, false, false);

		//Range: 98 (exclusive)  -  0 (inclusive)
		//Expected result: values 96 - 0 (inclusive)
		AssignString(startv, 98);
		AssignBound(startBound, false, startv);
		AssignRange(range, false, 500, &startBound);
		TestRange<int>(buf, range, 96, 0, 49, -2, false, true, false);


		//Two bounds - inclusive, non overlapping
		//Range: 2 (inclusive) - 96 (inclusive)
		//Expected result: values 2 - 96 (inclusive)
		AssignString(endv, 96);
		AssignBound(endBound, true, endv);
		AssignString(startv, 2);
		AssignBound(startBound, true, startv);		
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 2, 96, 48, 2, false, false, false);

		//Range: 96 (inclusive) - 2 (inclusive)
		//Expected result: values 96 - 2 (inclusive)
		AssignString(endv, 2);
		AssignBound(endBound, true, endv);
		AssignString(startv, 96);
		AssignBound(startBound, true, startv);		
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 96, 2, 48, -2, false, false, false);

		//Range: 3 (inclusive) - 95 (inclusive)
		//Expected result: values 4 - 94 (inclusive)
		AssignString(endv, 95);
		AssignBound(endBound, true, endv);
		AssignString(startv, 3);
		AssignBound(startBound, true, startv);		
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 4, 94, 46, 2, false, false, false);

		//Range: 95 (inclusive) - 3 (inclusive)
		//Expected result: values 94 - 4 (inclusive)
		AssignString(endv, 3);
		AssignBound(endBound, true, endv);
		AssignString(startv, 95);
		AssignBound(startBound, true, startv);		
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 94, 4, 46, -2, false, false, false);


		//Two bounds - exclusive, non overlapping
		//Range: 2 (exclusive) - 96 (exclusive)
		//Expected result: values 4 - 94 (inclusive)
		AssignString(endv, 96);
		AssignBound(endBound, false, endv);
		AssignString(startv, 2);
		AssignBound(startBound, false, startv);		
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 4, 94, 46, 2, false, false, false);

		//Range: 96 (exclusive) - 2 (exclusive)
		//Expected result: values 94 - 4 (inclusive)
		AssignString(endv, 2);
		AssignBound(endBound, false, endv);
		AssignString(startv, 96);
		AssignBound(startBound, false, startv);		
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 94, 4, 46, -2, false, false, false);

		//Range: 3 (exclusive) - 95 (exclusive)
		//Expected result: values 4 - 94 (exclusive)
		AssignString(endv, 95);
		AssignBound(endBound, false, endv);
		AssignString(startv, 3);
		AssignBound(startBound, false, startv);		
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 4, 94, 46, 2, false, false, false);

		//Range: 95 (exclusive) - 3 (exclusive)
		//Expected result: values 94 - 4 (inclusive)
		AssignString(endv, 3);
		AssignBound(endBound, false, endv);
		AssignString(startv, 95);
		AssignBound(startBound, false, startv);		
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 94, 4, 46, -2, false, false, false);


		//Two bounds - inclusive, overlapping
		//Range: 50 (inclusive) - 50 (inclusive) ascending
		//Expected result: 50
		AssignString(endv, 50);
		AssignBound(endBound, true, endv);
		AssignString(startv, 50);
		AssignBound(startBound, true, startv);
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 50, 50, 1, 0, false, false, false);

		//Range: 50 (inclusive) - 50 (inclusive) desc
		//Expected result: 50
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 50, 50, 1, 0, false, false, false);


		//Two bounds - exclusive, overlaping
		//Range: 50 (exclusive) - 50 (exclusive) asc
		//Expected result: Empty
		AssignString(endv, 50);
		AssignBound(endBound, true, endv);
		AssignString(startv, 50);
		AssignBound(startBound, false, startv);
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 0, 0, 0, 0, false, false, false);

		//Range: 50 (exclusive) - 50 (exclusive) desc
		//Expected result: Empty
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 0, 0, 0, 0, false, false, false);


		//Start after data - asc
		//Expected result: Empty
		AssignString(startv, 100);
		AssignBound(startBound, true, startv);
		AssignRange(range, true, 500, &startBound);
		TestRange<int>(buf, range, 0, 0, 0, 0, false, true, false);

		//Start after data - desc
		//Expected result: Empty
		AssignString(startv, -2);
		AssignBound(startBound, true, startv);
		AssignRange(range, false, 500, &startBound);
		TestRange<int>(buf, range, 0, 0, 0, 0, false, true, false);


		//Start at end
		//Range 98 (inclusive) - end asc
		//Expected result: 98
		AssignString(startv, 98);
		AssignBound(startBound, true, startv);
		AssignRange(range, true, 500, &startBound);
		TestRange<int>(buf, range, 98, 98, 1, 0, false, true, false);

		//Range start - 98
		//Expected result: 98 desc
		AssignString(endv, 98);
		AssignBound(endBound, true, endv);
		AssignRange(range, false, 500, NULL, &endBound);
		TestRange<int>(buf, range, 98, 98, 1, 0, true, false, false);


		//Range: 98 (exclusive) - end asc
		//Expected result: Empty
		AssignBound(startBound, false, startv);
		AssignRange(range, true, 500, &startBound);
		TestRange<int>(buf, range, 0, 0, 0, 0, false, true, false);

		//Range start - 98 (exclusive) desc
		//Expected result: Empty
		AssignString(endv, 98);
		AssignBound(endBound, false, endv);
		AssignRange(range, false, 500, NULL, &endBound);
		TestRange<int>(buf, range, 0, 0, 0, 0, true, false, false);


		//End before data - asc
		//Expected result: Empty
		AssignString(endv, -2);
		AssignBound(endBound, true, endv);
		AssignRange(range, true, 500, NULL, &endBound);
		TestRange<int>(buf, range, 0, 0, 0, 0, true, false, false);

		//End before data - desc
		//Expected result: Empty
		AssignString(endv, 100);
		AssignBound(endBound, true, endv);
		AssignRange(range, false, 500, NULL, &endBound);
		TestRange<int>(buf, range, 0, 0, 0, 0, true, false, false);


		//End at Start
		// Range: start - 0 (inclusive) asc
		//Expected result: 0
		AssignString(endv, 0);
		AssignBound(endBound, true, endv);
		AssignRange(range, true, 500, NULL, &endBound);
		TestRange<int>(buf, range, 0, 0, 1, 0, true, false, false);

		// Range: start - 98 (inclusive) desc
		//Expected result: 98
		AssignString(endv, 98);
		AssignBound(endBound, true, endv);
		AssignRange(range, false, 500, NULL, &endBound);
		TestRange<int>(buf, range, 98, 98, 1, 0, true, false, false);

		// Range: start - 0 (exclusive) asc
		//Expected result: Empty
		AssignString(endv, 0);
		AssignBound(endBound, false, endv);
		AssignRange(range, true, 500, NULL, &endBound);
		TestRange<int>(buf, range, 0, 0, 0, 0, true, false, false);

		// Range: start - 98 (exclusive) desc
		//Expected result: Empty
		AssignString(endv, 98);
		AssignBound(endBound, false, endv);
		AssignRange(range, false, 500, NULL, &endBound);
		TestRange<int>(buf, range, 0, 0, 0, 0, true, false, false);


		//Limited range
		//Range: start - end asc, limited to 5
		//Expected result: 0 - 8 (inclusive)
		AssignRange(range, true, 5);
		TestRange<int>(buf, range, 0, 8, 5, 2, true, false, true);

		//Range: end - start desc, limited to 5
		//Expected result: 98-90 (inclusive)
		AssignRange(range, false, 5);
		TestRange<int>(buf, range, 98, 90, 5, -2, true, false, true);

		//Start on ID  - 0
		//For a unique buffer there is one id per value, so a startID is essentially the same as using the exclusive flag.
		//Range: start - end asc, start on 0 ( 0 is excluded since it's assumed it's part of the last set)
		AssignString(startv, 0);
		AssignRange(range, true, 500, NULL, NULL, &startv);
		TestRange<int>(buf, range, 2, 98, 49, 2, false, true, false);

		//Range: end - start desc, start on 98 ( 0 is excluded since it's assumed it's part of the last set)
		AssignString(endv, 98);	
		AssignRange(range, false, 500, NULL, NULL, &endv);
		TestRange<int>(buf, range, 96, 0, 49, -2, false, true, false);

		//Start on ID  - 2
		//Range: start - end asc, start on 2
		AssignString(startv, 2);
		AssignBound(startBound, true, startv);
		AssignRange(range, true, 500, &startBound, NULL, &startv);
		TestRange<int>(buf, range, 2, 98, 49, 2, false, true, false);

		//Range: end - start desc, start on 98 (0 is excluded since it's assumed it's part of the last set)
		AssignString(endv, 98);
		AssignBound(endBound, true, endv);
		AssignRange(range, false, 500, NULL, &endBound, &endv);
		TestRange<int>(buf, range, 98, 2, 49, -2, true, false, false);

		//Combination - asc
		AssignString(endv, 94);
		AssignBound(endBound, false, endv);
		AssignString(startv, 80);
		AssignBound(startBound, false, startv);
		AssignRange(range, true, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 82, 92, 6, 2, false, false, false);

		AssignRange(range, true, 5, &startBound, &endBound);
		TestRange<int>(buf, range, 82, 90, 5, 2, false, false, true);

		//Combination - des
		AssignString(endv, 80);
		AssignBound(endBound, false, endv);
		AssignString(startv, 94);
		AssignBound(startBound, false, startv);
		AssignRange(range, false, 500, &startBound, &endBound);
		TestRange<int>(buf, range, 92, 82, 6, -2, false, false, false);

		AssignRange(range, false, 5, &startBound, &endBound);
		TestRange<int>(buf, range, 92, 84, 5, -2, false, false, true);



		//TODO: Have to use a new buffer for float types. 
		
		//Testing for float types
		//Insert values 0 - 24.5 (inclusive) in increments of 0.5 into buffer
		//for (float i = 0; i < 25; i += 0.5)
		//{
		//	Include inc;
		//	//TODO: Create thrift strings
		//	string rowId;
		//	AssignString(rowId, i);

		//	string value;
		//	AssignString(value, i);

		//	inc.__set_rowID(rowId);
		//	inc.__set_value(value);
		//	includes.push_back(inc); 
		//}

		//cw.__set_includes(includes);
		//buf.Apply(cw);

		//Assert::AreEqual<long long>(buf.GetStatistic().total, 50);
		//Assert::AreEqual<long long>(buf.GetStatistic().unique, 50);

		////Entire Set
		////Range: Entire set ascending
		////Expected result: values 0 - 24.5 (inclusive) by 0.5s.
		//AssignRange(range, true, 500);
		//TestRange<float>(buf, range, 0, 24.5, 50, 0.5, true, true, false);

		////Range: Entire set descending
		////Expected result: values 24.5 - 0 (inclusive) by -0.5s.
		//AssignRange(range, false, 500);
		//TestRange<float>(buf, range, 24.5, 0, 50, -0.5, true, true, false);


		////Start Exclusive - Non overlapping
		////Range: 0 (exclusive) - end (inclusive) 
		////Expected result: values 0.5 - 24.5 (inclusive)
		//AssignString(startv, 0);
		//AssignBound(startBound, false, startv);
		//AssignRange(range, true, 500, &startBound);
		//TestRange<float>(buf, range, 0.5, 24.5, 49, 0.5, false, true, false);

		////Range:  24.5 (inc) - 0 (exclusive) 
		////Expected result: values 24.5 - 0.5 (inclusive)
		//AssignString(endv, 0);
		//AssignBound(endBound, false, startv);
		//AssignRange(range, false, 500, NULL, &endBound);
		//TestRange<float>(buf, range, 24.5, 0.5, 49, -0.5, true, false, false);


		////End Exclusive - Non overlapping
		////Range: begin (inclusive) - 24.5 (exclusive)
		////Expected result: values 0 - 24 (inclusive)
		//AssignString(endv, 98);
		//AssignBound(endBound, false, endv);
		//AssignRange(range, true, 500, NULL, &endBound);
		//TestRange<float>(buf, range, 0, 24, 49, 0.5, true, false, false);

		////Range: 24.5 (exclusive)  -  0 (inclusive)
		////Expected result: values 24 - 0 (inclusive)
		//AssignString(startv, 98);
		//AssignBound(startBound, false, startv);
		//AssignRange(range, false, 500, &startBound);
		//TestRange<float>(buf, range, 24, 0, 49, -0.5, false, true, false);


		////Two bounds - inclusive, non overlapping
		////Range: 2 (inclusive) - 24 (inclusive)
		////Expected result: values 2 - 24 (inclusive)
		//AssignString(endv, 24);
		//AssignBound(endBound, true, endv);
		//AssignString(startv, 2);
		//AssignBound(startBound, true, startv);		
		//AssignRange(range, true, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 2, 24, 45, 0.5, false, false, false);

		////Range: 24 (inclusive) - 2 (inclusive)
		////Expected result: values 24 - 2 (inclusive)
		//AssignString(endv, 2);
		//AssignBound(endBound, true, endv);
		//AssignString(startv, 24);
		//AssignBound(startBound, true, startv);		
		//AssignRange(range, false, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 24, 2, 45, -0.5, false, false, false);

		////Range: 3.75 (inclusive) - 24.25 (inclusive)
		////Expected result: values 4 - 24 (inclusive)
		//AssignString(endv, 24.25);
		//AssignBound(endBound, true, endv);
		//AssignString(startv, 3.75);
		//AssignBound(startBound, true, startv);		
		//AssignRange(range, true, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 4, 24, 43, 0.5, false, false, false);

		////Range: 24.25 (inclusive) - 3.75 (inclusive)
		////Expected result: values 24 - 4 (inclusive)
		//AssignString(endv, 3);
		//AssignBound(endBound, true, endv);
		//AssignString(startv, 24.25);
		//AssignBound(startBound, true, startv);		
		//AssignRange(range, false, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 24, 4, 43, -0.5, false, false, false);

		//
		////Two bounds - exclusive, non overlapping
		////Range: 0 (exclusive) - 24.5 (exclusive)
		////Expected result: values 0.5 - 24 (inclusive)
		//AssignString(endv, 24.5);
		//AssignBound(endBound, false, endv);
		//AssignString(startv, 0);
		//AssignBound(startBound, false, startv);		
		//AssignRange(range, true, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 0.5, 24, 48, 0.5, false, false, false);

		////Range: 24.5 (exclusive) - 0 (exclusive)
		////Expected result: values 24 - 0.5 (inclusive)
		//AssignString(endv, 0);
		//AssignBound(endBound, false, endv);
		//AssignString(startv, 24.5);
		//AssignBound(startBound, false, startv);		
		//AssignRange(range, false, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 24, 0.5, 48, -0.5, false, false, false);

		////Range: 0.25 (exclusive) - 24.25 (exclusive)
		////Expected result: values 0.5 - 24 (inclusive)
		//AssignString(endv, 24.25);
		//AssignBound(endBound, false, endv);
		//AssignString(startv, 0.25);
		//AssignBound(startBound, false, startv);		
		//AssignRange(range, true, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 0.5, 24, 48, 0.5, false, false, false);

		////Range: 24.25 (exclusive) - 0.25 (exclusive)
		////Expected result: values 24 - 0.5 (inclusive)
		//AssignString(endv, 0.25);
		//AssignBound(endBound, false, endv);
		//AssignString(startv, 24.25);
		//AssignBound(startBound, false, startv);		
		//AssignRange(range, false, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 24, 0.5, 48, -0.5, false, false, false);


		////Two bounds - inclusive, overlapping
		////Range: 10 (inclusive) - 10 (inclusive) ascending
		////Expected result: 10
		//AssignString(endv, 10);
		//AssignBound(endBound, true, endv);
		//AssignString(startv, 10);
		//AssignBound(startBound, true, startv);
		//AssignRange(range, true, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 10, 10, 1, 0, false, false, false);

		////Range: 10 (inclusive) - 10 (inclusive) desc
		////Expected result: 10
		//AssignRange(range, false, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 10, 10, 1, 0, false, false, false);


		////Two bounds - exclusive, overlaping
		////Range: 10 (exclusive) - 10 (exclusive) asc
		////Expected result: Empty
		//AssignString(endv, 10);
		//AssignBound(endBound, true, endv);
		//AssignString(startv, 10);
		//AssignBound(startBound, false, startv);
		//AssignRange(range, true, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 0, 0, 0, 0, false, false, false);

		////Range: 10 (exclusive) - 10 (exclusive) desc
		////Expected result: Empty
		//AssignRange(range, false, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 0, 0, 0, 0, false, false, false);


		////Start after data - asc
		////Expected result: Empty
		//AssignString(startv, 25);
		//AssignBound(startBound, true, startv);
		//AssignRange(range, true, 500, &startBound);
		//TestRange<float>(buf, range, 0, 0, 0, 0, false, true, false);

		////Start after data - desc
		////Expected result: Empty
		//AssignString(startv, -2);
		//AssignBound(startBound, true, startv);
		//AssignRange(range, false, 500, &startBound);
		//TestRange<float>(buf, range, 0, 0, 0, 0, false, true, false);


		////Start at end
		////Range 24.5 (inclusive) - end asc
		////Expected result: 24.5
		//AssignString(startv, 24.5);
		//AssignBound(startBound, true, startv);
		//AssignRange(range, true, 500, &startBound);
		//TestRange<float>(buf, range, 24.5, 24.5, 1, 0, false, true, false);

		////Range start - 24.5
		////Expected result: 24.5 desc
		//AssignString(endv, 24.5);
		//AssignBound(endBound, true, endv);
		//AssignRange(range, false, 500, NULL, &endBound);
		//TestRange<float>(buf, range, 24.5, 24.5, 1, 0, true, false, false);


		////Range: 24.5 (exclusive) - end asc
		////Expected result: Empty
		//AssignBound(startBound, false, startv);
		//AssignRange(range, true, 500, &startBound);
		//TestRange<float>(buf, range, 0, 0, 0, 0, false, true, false);

		////Range start - 24.5 (exclusive) desc
		////Expected result: Empty
		//AssignString(endv, 24.5);
		//AssignBound(endBound, false, endv);
		//AssignRange(range, false, 500, NULL, &endBound);
		//TestRange<float>(buf, range, 0, 0, 0, 0, true, false, false);


		////End before data - asc
		////Expected result: Empty
		//AssignString(endv, -2);
		//AssignBound(endBound, true, endv);
		//AssignRange(range, true, 500, NULL, &endBound);
		//TestRange<float>(buf, range, 0, 0, 0, 0, true, false, false);

		////End before data - desc
		////Expected result: Empty
		//AssignString(endv, 25);
		//AssignBound(endBound, true, endv);
		//AssignRange(range, false, 500, NULL, &endBound);
		//TestRange<float>(buf, range, 0, 0, 0, 0, true, false, false);


		////End at Start
		//// Range: start - 0 (inclusive) asc
		////Expected result: 0
		//AssignString(endv, 0);
		//AssignBound(endBound, true, endv);
		//AssignRange(range, true, 500, NULL, &endBound);
		//TestRange<float>(buf, range, 0, 0, 1, 0, true, false, false);

		//// Range: start - 24.5 (inclusive) asc
		////Expected result: 24.5
		//AssignString(endv, 24.5);
		//AssignBound(endBound, true, endv);
		//AssignRange(range, false, 500, NULL, &endBound);
		//TestRange<float>(buf, range, 24.5, 24.5, 1, 0, true, false, false);

		//// Range: start - 0 (exclusive) desc
		////Expected result: Empty
		//AssignString(endv, 0);
		//AssignBound(endBound, false, endv);
		//AssignRange(range, true, 500, NULL, &endBound);
		//TestRange<float>(buf, range, 0, 0, 0, 0, true, false, false);

		//// Range: start - 24.5 (exclusive) desc
		////Expected result: Empty
		//AssignString(endv, 24.5);
		//AssignBound(endBound, false, endv);
		//AssignRange(range, false, 500, NULL, &endBound);
		//TestRange<float>(buf, range, 0, 0, 0, 0, true, false, false);


		////Limited range
		////Range: start - end asc, limited to 5
		////Expected result: 0 - 2 (inclusive)
		//AssignRange(range, true, 5);
		//TestRange<float>(buf, range, 0, 2, 5, 0.5, true, false, true);

		////Range: end - start desc, limited to 5
		////Expected result: 98-90 (inclusive)
		//AssignRange(range, false, 5);
		//TestRange<float>(buf, range, 24.5, 22.5, 5, -0.5, true, false, true);

		////Start on ID  - 0
		////For a unique buffer there is one id per value, so a startID is essentially the same as using the exclusive flag.
		////Range: start - end asc, start on 0 ( 0 is excluded since it's assumed it's part of the last set)
		//AssignString(startv, 0);
		//AssignRange(range, true, 500, NULL, NULL, &startv);
		//TestRange<float>(buf, range, 0.5, 24.5, 49, 0.5, false, true, false);

		////Range: end - start desc, start on 98 ( 0 is excluded since it's assumed it's part of the last set)
		//AssignString(endv, 24.5);	
		//AssignRange(range, false, 500, NULL, NULL, &endv);
		//TestRange<float>(buf, range, 24, 0, 49, -0.5, false, true, false);

		////Start on ID  - 0.5
		////Range: start - end asc, start on 0.5
		//AssignString(startv, 0.5);
		//AssignBound(startBound, true, startv);
		//AssignRange(range, true, 500, &startBound, NULL, &startv);
		//TestRange<float>(buf, range, 0.5, 24.5, 49, 0.5, false, true, false);

		////Range: end - start desc, start on 98 (0 is excluded since it's assumed it's part of the last set)
		//AssignString(endv, 24.5);
		//AssignBound(endBound, true, endv);
		//AssignRange(range, false, 500, NULL, &endBound, &endv);
		//TestRange<float>(buf, range, 24.5, 0.5, 49, -0.5, true, false, false);

		////Combination - asc
		//AssignString(endv, 24);
		//AssignBound(endBound, false, endv);
		//AssignString(startv, 21);
		//AssignBound(startBound, false, startv);
		//AssignRange(range, true, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 21.5, 23.5, 5, 0.5, false, false, false);

		//AssignRange(range, true, 3, &startBound, &endBound);
		//TestRange<float>(buf, range, 21.5, 22.5, 3, 0.5, false, false, true);

		////Combination - des
		//AssignString(endv, 21);
		//AssignBound(endBound, false, endv);
		//AssignString(startv, 24);
		//AssignBound(startBound, false, startv);
		//AssignRange(range, false, 500, &startBound, &endBound);
		//TestRange<float>(buf, range, 23.5, 21.5, 5, -0.5, false, false, false);

		//AssignRange(range, false, 3, &startBound, &endBound);
		//TestRange<float>(buf, range, 23.5, 22.5, 3, -0.5, false, false, true);

	}

	template <typename Type> void TestRange(IdentityBuffer& buf, RangeRequest range, Type expectedStart, Type expectedEnd, int expectedValuesCount, int increment, bool expectBOF, bool expectEOF, bool expectLimited)
	{
		RangeResult result = buf.GetRows(range);
		
		//Right number of values...
		Assert::AreEqual<int>(result.valueRowsList.size(), expectedValuesCount);

		Type expectedNum = expectedStart;
		for (int i = 0; i < result.valueRowsList.size(); i++)
		{
			//Item is a value-rows  A - 1, 2, 3
			auto item = result.valueRowsList[i];

			auto value = *(Type*)item.value.data();

			//Value should be our expected number;
			Assert::AreEqual<Type>(expectedNum, value);

			//Value should be our expected type
			//TODO: This is isn't needed. We've made a cast, so we are by defnition in the right type.
			//(The cast may not have been valid, but hopefully we'll pick that up since we are checking for the correct value.)
			//Assert::AreEqual<Type>(typeid(expectedNum), typeid(value));

			//should be one id per value (unique buffer)
			Assert::IsTrue(item.rowIDs.size() == 1);

			//id should be expected type
			//Assert::AreEqual<Type>(typeid(expectedNum), typeid(*(Type*)item.rowIDs[0].data()));

			//id should be expected number
			//Assert::AreEqual<Type>(expectedNum, *(Type*)item.rowIDs[0].data());		

			expectedNum += increment;
		}

		//We should see the expectedEnd + increment if we've iterated all values, or just expected end if we didn't iterate.
		Assert::AreEqual<int>(expectedNum, expectedEnd + (result.valueRowsList.size() > 0 ? increment : 0));
		Assert::IsTrue(result.bof == expectBOF);
		Assert::IsTrue(result.eof == expectEOF);
		Assert::IsTrue(result.limited == expectLimited);
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
};