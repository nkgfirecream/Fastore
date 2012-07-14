#include "stdafx.h"
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "Column\IdentityBuffer.h"

using namespace std;


TEST_CLASS(IdentityBufferTest)
{
public:
	
	TEST_METHOD(IdentityRangeTests)
	{
		
		//TODO: Update limited behavior to reflect BoF/EoF semantics.
		//Unique buffer -- one key has one and only one value
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

	void TestRange(IdentityBuffer& buf, RangeRequest range, int expectedStart, int expectedEnd, int expectedValuesCount, int increment, bool expectBOF, bool expectEOF, bool expectLimited)
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