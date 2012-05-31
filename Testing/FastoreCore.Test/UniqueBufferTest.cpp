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
		TestRange(buf, range, 0, 98, 50, 2, true, true, false);

		//Range: Entire set descending
		//Expected result: values 98 - 0 (inclusive) by -2s.
		range = Range(500, false);
		TestRange(buf, range, 98, 0, 50, -2, true, true, false);


		//Start Exclusive - Non overlapping
		//Range: 0 (exclusive) - end (inclusive) ascending
		//Expected result: values 2 - 98 (inclusive)
		int startValue = 0;
		RangeBound startBound(&startValue, false);
		range = Range(500, true, startBound);
		TestRange(buf, range, 2, 98, 49, 2, false, true, false);

		//Range: 0 (exclusive) - end (inclusive) descending
		//Expected result: values 98 - 2 (inclusive)
		range = Range(500, false, startBound);
		TestRange(buf, range, 98, 2, 49, -2, false, true, false);


		//End Exclusive - Non overlapping
		//Range: begin (inclusive) - 98 (exclusive) ascending
		//Expected result: values 0 - 96 (inclusive)
		int endValue = 98;
		RangeBound endBound(&endValue, false);
		range = Range(500, true, Optional<fs::RangeBound>(), endBound);
		TestRange(buf, range, 0, 96, 49, 2, true, false, false);

		//Range: begin (inclusive) - 98 (exclusive) descending
		//Expected result: values 96 - 0 (inclusive)
		range = Range(500, false, Optional<fs::RangeBound>(), endBound);
		TestRange(buf, range, 96, 0, 49, -2, true, false, false);

		
		//Two bounds - inclusive, non overlapping
		//Range: 2 (inclusive) - 96 (inclusive) ascending
		//Expected result: values 2 - 96 (inclusive)
		endValue = 96;
		endBound = RangeBound(&endValue, true);
		startValue = 2;
		startBound = RangeBound(&startValue, true);
		range = Range(500, true, startBound, endBound);
		TestRange(buf, range, 2, 96, 48, 2, false, false, false);

		//Range: 2 (inclusive) - 96 (inclusive) descending
		//Expected result: values 96 - 2 (inclusive)
		range = Range(500, false, startBound, endBound);
		TestRange(buf, range, 96, 2, 48, -2, false, false, false);


		//Two bounds - exclusive, non overlapping
		//Range: 2 (exclusive) - 96 (exclusive) ascending
		//Expected result: values 4 - 94 (inclusive)
		endValue = 96;
		endBound = RangeBound(&endValue, false);
		startValue = 2;
		startBound = RangeBound(&startValue, false);
		range = Range(500, true, startBound, endBound);
		TestRange(buf, range, 4, 94, 46, 2, false, false, false);

		//Range: 2 (exclusive) - 96 (exclusive) descending
		//Expected result: values 94 - 4 (inclusive)
		range = Range(500, false, startBound, endBound);
		TestRange(buf, range, 94, 4, 46, -2, false, false, false);

		
		//Two bounds - inclusive, overlapping
		//Range: 50 (inclusive) - 50 (inclusive) ascending
		//Expected result: 50
		endValue = 50;
		startValue = 50;
		startBound = RangeBound(&startValue, true);
		endBound = RangeBound(&endValue, true);
		range = Range(500, true, startBound, endBound);
		TestRange(buf, range, 50, 50, 1, 0, false, false, false);

		//Range: 50 (inclusive) - 50 (inclusive) desc
		//Expected result: 50
		range = Range(500, false, startBound, endBound);
		TestRange(buf, range, 50, 50, 1, 0, false, false, false);


		//Two bounds - exclusive, overlaping
		//Range: 50 (exclusive) - 50 (exclusive) asc
		//Expected result: Empty
		endValue = 50;
		startValue = 50;
		startBound = RangeBound(&startValue, false);
		endBound = RangeBound(&endValue, false);
		range = Range(500, true, startBound, endBound);
		TestRange(buf, range, 0, 0, 0, 0, false, false, false);

		//Range: 50 (exclusive) - 50 (exclusive) desc
		//Expected result: Empty
		range = Range(500, false, startBound, endBound);
		TestRange(buf, range, 0, 0, 0, 0, false, false, false);


		//Start after data - asc
		//Expected result: Empty
		startValue = 100;
		startBound = RangeBound(&startValue, true);
		range = Range(500, true, startBound);
		TestRange(buf, range, 0, 0, 0, 0, false, true, false);

		//Start after data - desc
		//Expected result: Empty
		range = Range(500, false, startBound);
		TestRange(buf, range, 0, 0, 0, 0, false, true, false);


		//Start at end
		//Range 98 (inclusive) - end asc
		//Expected result: 98
		startValue = 98;
		startBound = RangeBound(&startValue, true);
		range = Range(500, true, startBound);
		TestRange(buf, range, 98, 98, 1, 0, false, true, false);

		//Range 98 (inclusive) - end asc
		//Expected result: 98
		range = Range(500, false, startBound);
		TestRange(buf, range, 98, 98, 1, 0, false, true, false);

		//Range: 98 (exclusive) - end asc
		//Expected result: Empty
		startBound = RangeBound(&startValue, false);
		range = Range(500, true, startBound);
		TestRange(buf, range, 0, 0, 0, 0, false, true, false);

		//Range 98 (exclusive) - end desc
		//Expected result: Empty
		range = Range(500, false, startBound);
		TestRange(buf, range, 0, 0, 0, 0, false, true, false);
		

		//End before data - asc
		//Expected result: Empty
		endValue = -2;
		endBound = RangeBound(&endValue, true);
		range = Range(500, true, Optional<fs::RangeBound>(), endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);

		//End before data - desc
		//Expected result: Empty
		endBound = RangeBound(&endValue, true);
		range = Range(500, true, Optional<fs::RangeBound>(), endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);

		
		//End at Start
		// Range: start - 0 (inclusive) asc
		//Expected result: 0
		endValue = 0;
		endBound = RangeBound(&endValue, true);
		range = Range(500, true, Optional<fs::RangeBound>(), endBound);
		TestRange(buf, range, 0, 0, 1, 0, true, false, false);

		// Range: start - 0 (inclusive) desc
		//Expected result: 0
		range = Range(500, false, Optional<fs::RangeBound>(), endBound);
		TestRange(buf, range, 0, 0, 1, 0, true, false, false);

		// Range: start - 0 (exclusive) asc
		//Expected result: Empty
		endBound = RangeBound(&endValue, false);
		range = Range(500, true, Optional<fs::RangeBound>(), endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);

		// Range: start - 0 (exclusive) desc
		//Expected result: Empty
		range = Range(500, false, Optional<fs::RangeBound>(), endBound);
		TestRange(buf, range, 0, 0, 0, 0, true, false, false);

		
		//Limited range
		//Range: start - end asc, limited to 5
		//Expected result: 0 - 8 (inclusive)
		range = Range(5, true);
		TestRange(buf, range, 0, 8, 5, 2, true, false, true);

		//Range: end - start desc, limited to 5
		//Expected result: 98-90 (inclusive)
		range = Range(5,false);
		TestRange(buf, range, 98, 90, 5, -2, false, true, true);

		//Start on ID
		//For a unique buffer there is one id per value, so a startID is essentially the same as using the exclusive flag.
		//Range: start - end asc, start on 0 ( 0 is excluded since it's assumed it's part of the last set)
		startValue = 0;
		startBound = RangeBound(&startValue, true, Optional<void*>(&startValue));
		range = Range(500, true, startBound);
		TestRange(buf, range, 2, 98, 49, 2, false, true, false);

		//Range: end - start desc, start on 0 ( 0 is excluded since it's assumed it's part of the last set)
		endValue = 98;
		endBound = RangeBound(&endValue, true, Optional<void*>(&endValue));
		range = Range(500, false, Optional<RangeBound>(), endBound);
		TestRange(buf, range, 96, 0, 49, -2, true, false, false);


		//Combination
		endValue = 94;
		startValue = 80;
		startBound = RangeBound(&startValue, true, Optional<void*>(&startValue));
		endBound = RangeBound(&endValue, false);
		range = Range(500, true, startBound, endBound);
		TestRange(buf, range, 82, 92, 6, 2, false, false, false);

		range = Range(5, true, startBound, endBound);
		TestRange(buf, range, 82, 90, 5, 2, false, false, true);
	}

	void TestRange(UniqueBuffer& buf, Range range, int expectedStart, int expectedEnd, int expectedValuesCount, int increment, bool expectBOF, bool expectEOF, bool expectLimited)
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
		CFIX_ASSERT(result.BeginOfFile == expectBOF);
		CFIX_ASSERT(result.EndOfFile == expectEOF);
		CFIX_ASSERT(result.Limited == expectLimited);
	}

	void IncludeExclude()
	{
		UniqueBuffer buf(standardtypes::Int, standardtypes::Int);

		bool result;
		int v = 0;
		int k = 0;

		//Non existing inserts should be ok
		result = buf.Include(&v, &k);
		CFIX_ASSERT(result == true);

		//Duplicate insertions should not insert
		result = buf.Include(&v, &k);
		CFIX_ASSERT(result == false);

		//Duplicate keys should not insert
		v = 2;
	    k = 0;
		result = buf.Include(&v, &k);
		CFIX_ASSERT(result == false);

		//Duplicate values should not insert
		v = 0;
	    k = 2;
		result = buf.Include(&v, &k);
		CFIX_ASSERT(result == false);

		//End of this should still be zero
		k = 0;
		void* value = buf.GetValue(&k);
		CFIX_ASSERT(*(int*)value == 0);

		//Values not present should not exclude
		v = 1;
		k = 0;
		result = buf.Exclude(&v, &k);
		CFIX_ASSERT(result == false);

		//Keys not present should not exclude
		v = 0;
		k = 1;
		result = buf.Exclude(&k);
		CFIX_ASSERT(result == false);

		result = buf.Exclude(&v, &k);
		CFIX_ASSERT(result == false);

		//Keys present should exclude
		v = 0;
		k = 0;
		result = buf.Exclude(&v, &k);
		CFIX_ASSERT(result == true);

		//A bunch of insertions should work...
		int numrows = 10000;
		for (int i = 0; i <= numrows; i++)
		{
			result = buf.Include(&i,&i);
			CFIX_ASSERT(result == true);
		}

		//A bunch of exclusions should work...
		for (int i = numrows / 2; i <= numrows; i++)
		{
			result = buf.Exclude(&i,&i);
			CFIX_ASSERT(result == true);
		}

		//All the values should still be the same...
		for (int i = 0; i < numrows / 2; i++)
		{
			value = buf.GetValue(&i);
			CFIX_ASSERT(*(int*)value == i);
		}
	}
};

CFIXCC_BEGIN_CLASS( UniqueBufferTest )
	CFIXCC_METHOD( RangeTests )
	CFIXCC_METHOD( IncludeExclude )
CFIXCC_END_CLASS()