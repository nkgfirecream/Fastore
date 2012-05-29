#include "StdAfx.h"
#include <cfixcc.h>

#include <sstream>
#include <iostream>
#include <hash_set>
#include <algorithm>

#include "Schema\standardtypes.h"
#include "Column\UniqueBuffer.h"

using namespace std;


class UniqueBufferTest : public cfixcc::TestFixture
{
public:
	
	void RangeTests()
	{
		//TODO: Update limited behavior to reflect BoF/EoF semantics.
		//Unique buffer -- one key has one and only one value
		UniqueBuffer buf(standardtypes::Int, standardtypes::Int);

		//Insert values 0 - 98 (inclusive) in increments of 2 into buffer
		for (int i = 0; i < 100; i += 2)
		{
			buf.Include(&i, &i);
		}

		CFIX_ASSERT(buf.GetStatistics().Total == 50);
		CFIX_ASSERT(buf.GetStatistics().Unique == 50);

		//Entire Set
		//Range: Entire set ascending
		//Expected result: values 0 - 98 (inclusive) by 2s.
		Range range(500, true);
		TestRange(buf, range, 0, 98, 50, 2);

		//Range: Entire set descending
		//Expected result: values 98 - 0 (inclusive) by -2s.
		range = Range(500, false);
		TestRange(buf, range, 98, 0, 50, -2);


		//Start Exclusive - Non overlapping
		//Range: 0 (exclusive) - end (inclusive) ascending
		//Expected result: values 2 - 98 (inclusive)
		int startValue = 0;
		RangeBound startBound(&startValue, false);
		range = Range(500, true, startBound);
		TestRange(buf, range, 2, 98, 49, 2);

		//Range: 0 (exclusive) - end (inclusive) descending
		//Expected result: values 98 - 2 (inclusive)
		range = Range(500, false, startBound);
		TestRange(buf, range, 98, 2, 49, -2);


		//End Exclusive - Non overlapping
		//Range: begin (inclusive) - 98 (exclusive) ascending
		//Expected result: values 0 - 96 (inclusive)
		int endValue = 98;
		RangeBound endBound(&endValue, false);
		range = Range(500, true, Optional<fs::RangeBound>(), endBound);
		TestRange(buf, range, 0, 96, 49, 2);

		//Range: begin (inclusive) - 98 (exclusive) descending
		//Expected result: values 96 - 0 (inclusive)
		range = Range(500, false, Optional<fs::RangeBound>(), endBound);
		TestRange(buf, range, 96, 0, 49, -2);

		
		//Two bounds - inclusive, non overlapping
		//Range: 2 (inclusive) - 96 (inclusive) ascending
		//Expected result: values 2 - 96 (inclusive)
		endValue = 96;
		endBound = RangeBound(&endValue, true);
		startValue = 2;
		startBound = RangeBound(&startValue, true);
		range = Range(500, true, startBound, endBound);
		TestRange(buf, range, 2, 96, 48, 2);

		//Range: 2 (inclusive) - 96 (inclusive) descending
		//Expected result: values 96 - 2 (inclusive)
		range = Range(500, false, startBound, endBound);
		TestRange(buf, range, 96, 2, 48, -2);


		//Two bounds - exclusive, non overlapping
		//Range: 2 (exclusive) - 96 (exclusive) ascending
		//Expected result: values 4 - 94 (inclusive)
		endValue = 96;
		endBound = RangeBound(&endValue, false);
		startValue = 2;
		startBound = RangeBound(&startValue, false);
		range = Range(500, true, startBound, endBound);
		TestRange(buf, range, 4, 94, 46, 2);

		//Range: 2 (exclusive) - 96 (exclusive) descending
		//Expected result: values 94 - 4 (inclusive)
		range = Range(500, false, startBound, endBound);
		TestRange(buf, range, 94, 4, 46, -2);

		
		//Two bounds - inclusive, overlapping
		//Range: 50 (inclusive) - 50 (inclusive) ascending
		//Expected result: 50
		endValue = 50;
		startValue = 50;
		startBound = RangeBound(&startValue, true);
		endBound = RangeBound(&endValue, true);
		range = Range(500, true, startBound, endBound);
		TestRange(buf, range, 50, 50, 1, 0);

		//Range: 50 (inclusive) - 50 (inclusive) desc
		//Expected result: 50
		range = Range(500, false, startBound, endBound);
		TestRange(buf, range, 50, 50, 1, 0);


		//Two bounds - exclusive, overlaping
		//Range: 50 (exclusive) - 50 (exclusive) asc
		//Expected result: Empty
		endValue = 50;
		startValue = 50;
		startBound = RangeBound(&startValue, false);
		endBound = RangeBound(&endValue, false);
		range = Range(500, true, startBound, endBound);
		TestRange(buf, range, 0, 0, 0, 0);

		//Range: 50 (exclusive) - 50 (exclusive) desc
		//Expected result: Empty
		range = Range(500, false, startBound, endBound);
		TestRange(buf, range, 0, 0, 0, 0);


		//Start after data - asc
		//Expected result: Empty
		startValue = 100;
		startBound = RangeBound(&startValue, true);
		range = Range(500, true, startBound);
		TestRange(buf, range, 0, 0, 0, 0);

		//Start after data - asc
		//Expected result: Empty
		range = Range(500, false, startBound);
		TestRange(buf, range, 0, 0, 0, 0);


		//Start at end
		//Range 98 (inclusive) - end asc
		//Expected result: 98
		startValue = 98;
		startBound = RangeBound(&startValue, true);
		range = Range(500, true, startBound);
		TestRange(buf, range, 98, 98, 1, 0);

		//Range 98 (inclusive) - end asc
		//Expected result: 98
		range = Range(500, false, startBound);
		TestRange(buf, range, 98, 98, 1, 0);

		//Range: 98 (exclusive) - end asc
		//Expected result: Empty
		startBound = RangeBound(&startValue, false);
		range = Range(500, true, startBound);
		TestRange(buf, range, 0, 0, 0, 0);

		//Range 98 (exclusive) - end desc
		//Expected result: Empty
		range = Range(500, false, startBound);
		TestRange(buf, range, 0, 0, 0, 0);



		//End before data - asc
		//Expected result: Empty
		endValue = -2;
	}

	void TestRange(UniqueBuffer& buf, Range range, int expectedStart, int expectedEnd, int expectedValuesCount, int increment)
	{
		GetResult result = buf.GetRows(range);

		//Right number of values...
		CFIX_ASSERT(result.Data.size() == expectedValuesCount);

		int expectedNum = expectedStart;
		for (int i = 0; i < result.Data.size(); i++)
		{
			auto item = result.Data[i];
			//Value should be our expected number;
			CFIX_ASSERT(expectedNum == *(int*)item.first);

			//should be one id per value (unique buffer)
			CFIX_ASSERT(item.second.size() == 1);

			//id should be expected number
			CFIX_ASSERT(expectedNum == *(int*)item.second[0]);

			expectedNum += increment;
		}

		//We should see the expectedEnd + increment if we've iterated all values, or just expected end if we didn't iterate.
		CFIX_ASSERT(expectedNum == expectedEnd + (result.Data.size() > 0 ? increment : 0));
	}
};

CFIXCC_BEGIN_CLASS( UniqueBufferTest )
	CFIXCC_METHOD( RangeTests )
CFIXCC_END_CLASS()