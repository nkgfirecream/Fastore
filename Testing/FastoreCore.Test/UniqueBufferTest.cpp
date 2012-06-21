#include "stdafx.h"
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <sstream>
#include <iostream>
#include <hash_set>
#include <algorithm>

#include "Schema\standardtypes.h"
#include "Column\UniqueBuffer.h"

using namespace std;


TEST_CLASS(UniqueBufferTest)
{
public:
	
	TEST_METHOD(RangeTests)
	{
		
		//TODO: Update limited behavior to reflect BoF/EoF semantics.
		//Unique buffer -- one key has one and only one value
		UniqueBuffer buf(standardtypes::Int, standardtypes::Int);

		ColumnWrites cw;
		std:vector<Include> includes;

		//Insert values 0 - 98 (inclusive) in increments of 2 into buffer
		for (int i = 0; i < 100; i += 2)
		{
			Include inc;
			//TODO: Create thrift strings
			string rowId;
			rowId.assign((const char*)&i, sizeof(int));

			string value;
			value.assign((const char*)&i, sizeof(int));

			inc.__set_rowID(rowId);
			inc.__set_value(value);
			includes.push_back(inc); 
		}

		cw.__set_includes(includes);
		buf.Apply(cw);

		Assert::AreEqual<long long>(buf.GetStatistic().total, 50);
		Assert::AreEqual<long long>(buf.GetStatistic().unique, 50);

		//Entire Set
		//Range: Entire set ascending
		//Expected result: values 0 - 98 (inclusive) by 2s.
		RangeRequest range;
		range.__set_limit(500);
		range.__set_ascending(true);
		TestRange(buf, range, 0, 98, 50, 2, true, true, false);

		//Range: Entire set descending
		//Expected result: values 98 - 0 (inclusive) by -2s.
		range = RangeRequest();
		range.__set_limit(500);
		range.__set_ascending(false);
		TestRange(buf, range, 98, 0, 50, -2, true, true, false);


		//Start Exclusive - Non overlapping
		//Range: 0 (exclusive) - end (inclusive) ascending
		//Expected result: values 2 - 98 (inclusive)
		range.__set_limit(500);
		range.__set_ascending(true);
		int startBoundValue = 500;
		string startv;
		
		Assign(startv, startBoundValue);
		
		RangeBound bound;
		bound.__set_inclusive(false);
		bound.__set_value(startv);
		
		range.__set_first(bound);
		TestRange(buf, range, 2, 98, 49, 2, false, true, false);

		//Range: 0 (exclusive) - end (inclusive) descending
		//Expected result: values 98 - 2 (inclusive)
		range.__set_limit(500);
		range.__set_ascending(false);
		startBoundValue = 500;
		startv = string();

		Assign(startv, startBoundValue);

		bound = RangeBound();
		bound.__set_inclusive(false);
		bound.__set_value(startv);
		
		range.__set_first(bound);
		TestRange(buf, range, 98, 2, 49, -2, false, true, false);


		//End Exclusive - Non overlapping
		//Range: begin (inclusive) - 98 (exclusive) ascending
		//Expected result: values 0 - 96 (inclusive)
		int endValue = 98;
		string endv;

		Assign(endv, endValue);

		RangeBound endBound;
		endBound.__set_inclusive(false);
		endBound.__set_value(endv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_last(endBound);
		TestRange(buf, range, 0, 96, 49, 2, true, false, false);

		//Range: begin (inclusive) - 98 (exclusive) descending
		//Expected result: values 96 - 0 (inclusive)
		range.__set_limit(500);
		range.__set_ascending(false);
		range.__set_last(endBound);
		TestRange(buf, range, 96, 0, 49, -2, true, false, false);

		
		//Two bounds - inclusive, non overlapping
		//Range: 2 (inclusive) - 96 (inclusive) ascending
		//Expected result: values 2 - 96 (inclusive)
		endValue = 96;
		Assign(endv, endValue);

		endBound = RangeBound();
		endBound.__set_inclusive(true);
		endBound.__set_value(endv);

		int startValue = 2;
		Assign(startv, startValue);
		
		RangeBound startBound;
		startBound.__set_inclusive(true);
		startBound.__set_value(startv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_first(startBound);
		range.__set_last(endBound);
		TestRange(buf, range, 2, 96, 48, 2, false, false, false);

		//Range: 2 (inclusive) - 96 (inclusive) descending
		//Expected result: values 96 - 2 (inclusive)
		range.__set_limit(500);
		range.__set_ascending(false);
		range.__set_first(startBound);
		range.__set_last(endBound);
		TestRange(buf, range, 96, 2, 48, -2, false, false, false);


		//Two bounds - exclusive, non overlapping
		//Range: 2 (exclusive) - 96 (exclusive) ascending
		//Expected result: values 4 - 94 (inclusive)
		endValue = 96;
		Assign(endv, endValue);

		endBound = RangeBound();
		endBound.__set_inclusive(false);
		endBound.__set_value(endv);

		startValue = 2;
		Assign(startv, startValue);

		startBound = RangeBound();
		startBound.__set_inclusive(false);
		startBound.__set_value(startv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_first(startBound);
		range.__set_last(endBound);
		TestRange(buf, range, 4, 94, 46, 2, false, false, false);

		//Range: 2 (exclusive) - 96 (exclusive) descending
		//Expected result: values 94 - 4 (inclusive)
		range.__set_limit(500);
		range.__set_ascending(false);
		range.__set_first(startBound);
		range.__set_last(endBound);
		TestRange(buf, range, 94, 4, 46, -2, false, false, false);

		
		//Two bounds - inclusive, overlapping
		//Range: 50 (inclusive) - 50 (inclusive) ascending
		//Expected result: 50
		endValue = 50;
		Assign(endv, endValue);

		endBound = RangeBound();
		endBound.__set_inclusive(true);
		endBound.__set_value(endv);

		startValue = 50;
		Assign(startv, startValue);

		startBound = RangeBound();
		startBound.__set_inclusive(true);
		endBound.__set_value(startv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_first(startBound);
		range.__set_last(endBound);
		TestRange(buf, range, 50, 50, 1, 0, false, false, false);

		//Range: 50 (inclusive) - 50 (inclusive) desc
		//Expected result: 50
		range.__set_limit(500);
		range.__set_ascending(false);
		range.__set_first(startBound);
		range.__set_last(endBound);
		TestRange(buf, range, 50, 50, 1, 0, false, false, false);


		//Two bounds - exclusive, overlaping
		//Range: 50 (exclusive) - 50 (exclusive) asc
		//Expected result: Empty
		endValue = 50;
		Assign(endv, endValue);

		endBound = RangeBound();
		endBound.__set_inclusive(true);
		endBound.__set_value(endv);

		startValue = 50;
		Assign(startv, startValue);

		startBound = RangeBound();
		startBound.__set_inclusive(false);
		startBound.__set_value(startv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_first(startBound);
		range.__set_last(endBound);
		TestRange(buf, range, 0, 0, 0, 0, false, false, false);

		//Range: 50 (exclusive) - 50 (exclusive) desc
		//Expected result: Empty
		range.__set_limit(500);
		range.__set_ascending(false);
		range.__set_first(startBound);
		range.__set_last(endBound);
		TestRange(buf, range, 0, 0, 0, 0, false, false, false);


		//Start after data - asc
		//Expected result: Empty
		startValue = 100;
		Assign(startv, startValue);

		startBound = RangeBound();
		startBound.__set_inclusive(true);
		startBound.__set_value(startv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_first(startBound);
		TestRange(buf, range, 0, 0, 0, 0, false, true, false);

		//Start after data - desc
		//Expected result: Empty
		range.__set_limit(500);
		range.__set_ascending(false);
		range.__set_first(startBound);
		TestRange(buf, range, 0, 0, 0, 0, false, true, false);


		//Start at end
		//Range 98 (inclusive) - end asc
		//Expected result: 98
		startValue = 98;
		Assign(startv, startValue);

		startBound = RangeBound();
		startBound.__set_inclusive(true);
		startBound.__set_value(startv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_first(startBound);
		TestRange(buf, range, 98, 98, 1, 0, false, true, false);

		//Range 98 (inclusive) - end asc
		//Expected result: 98
		range.__set_limit(500);
		range.__set_ascending(false);
		range.__set_first(startBound);
		TestRange(buf, range, 98, 98, 1, 0, false, true, false);

		//Range: 98 (exclusive) - end asc
		//Expected result: Empty
		startBound = RangeBound();
		startBound.__set_inclusive(false);
		startBound.__set_value(startv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_first(startBound);
		TestRange(buf, range, 0, 0, 0, 0, false, true, false);

		//Range 98 (exclusive) - end desc
		//Expected result: Empty
		range.__set_limit(500);
		range.__set_ascending(false);
		range.__set_first(startBound);
		TestRange(buf, range, 0, 0, 0, 0, false, true, false);
		

		//End before data - asc
		//Expected result: Empty
		endValue = -2;
		Assign(endv, endValue);

		endBound = RangeBound();
		endBound.__set_inclusive(true);
		endBound.__set_value(endv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_last(endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);

		//End before data - desc
		//Expected result: Empty
		endBound = RangeBound();
		endBound.__set_inclusive(true);
		endBound.__set_value(endv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_last(endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);

		
		//End at Start
		// Range: start - 0 (inclusive) asc
		//Expected result: 0
		endValue = -2;
		Assign(endv, endValue);

		endBound = RangeBound();
		endBound.__set_inclusive(true);
		endBound.__set_value(endv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_last(endBound);
		TestRange(buf, range, 0, 0, 1, 0, true, false, false);

		// Range: start - 0 (inclusive) desc
		//Expected result: 0
		range.__set_limit(500);
		range.__set_ascending(false);
		range.__set_last(endBound);
		TestRange(buf, range, 0, 0, 1, 0, true, false, false);

		// Range: start - 0 (exclusive) asc
		//Expected result: Empty
		endBound = RangeBound();
		endBound.__set_inclusive(false);
		endBound.__set_value(endv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_last(endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);

		// Range: start - 0 (exclusive) desc
		//Expected result: Empty
		range.__set_limit(500);
		range.__set_ascending(false);
		range.__set_last(endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);

		
		//Limited range
		//Range: start - end asc, limited to 5
		//Expected result: 0 - 8 (inclusive)
		range.__set_limit(5);
		range.__set_ascending(true);
		TestRange(buf, range, 0, 8, 5, 2, true, false, true);

		//Range: end - start desc, limited to 5
		//Expected result: 98-90 (inclusive)
		range.__set_limit(5);
		range.__set_ascending(false);
		TestRange(buf, range, 98, 90, 5, -2, false, true, true);

		//Start on ID
		//For a unique buffer there is one id per value, so a startID is essentially the same as using the exclusive flag.
		//Range: start - end asc, start on 0 ( 0 is excluded since it's assumed it's part of the last set)
		startValue = 0;
		Assign(startv, startValue);

		startBound = RangeBound();
		startBound.__set_inclusive(true);
		startBound.__set_value(startv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_first(startBound);
		TestRange(buf, range, 2, 98, 49, 2, false, true, false);

		//Range: end - start desc, start on 0 ( 0 is excluded since it's assumed it's part of the last set)
		endValue = 98;
		Assign(endv, endValue);

		endBound = RangeBound();
		endBound.__set_inclusive(true);
		endBound.__set_value(endv);

		range.__set_limit(500);
		range.__set_ascending(false);
		range.__set_last(endBound);
		TestRange(buf, range, 96, 0, 49, -2, true, false, false);


		//Combination
		endValue = 94;
		Assign(endv, endValue);

		endBound = RangeBound();
		endBound.__set_inclusive(false);
		endBound.__set_value(endv);

		startValue = 80;
		Assign(startv, startValue);

		startBound = RangeBound();
		startBound.__set_inclusive(true);
		startBound.__set_value(startv);

		range.__set_limit(500);
		range.__set_ascending(true);
		range.__set_first(startBound);
		range.__set_last(endBound);
		TestRange(buf, range, 82, 92, 6, 2, false, false, false);

		range.__set_limit(5);
		range.__set_ascending(true);
		range.__set_first(startBound);
		range.__set_last(endBound);
		TestRange(buf, range, 82, 90, 5, 2, false, false, true);
	}

	void TestRange(UniqueBuffer& buf, RangeRequest range, int expectedStart, int expectedEnd, int expectedValuesCount, int increment, bool expectBOF, bool expectEOF, bool expectLimited)
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
		Assert::IsTrue(result.beginOfRange == expectBOF);
		Assert::IsTrue(result.endOfRange == expectEOF);
		Assert::IsTrue(result.limited == expectLimited);
	}

	TEST_METHOD(IncludeExclude)
	{
		throw "Not yet implemented";
		//UniqueBuffer buf(standardtypes::Int, standardtypes::Int);

	//	bool result;
	//	int v = 0;
	//	int k = 0;

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

		////Duplicate values should not insert
		//v = 0;
	 //   k = 2;
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

	void Assign(string& str, int value){
	 str.assign((const char*)&value, sizeof(int));
	}
};