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

		//Range: Entire set ascending
		//Expected result: values 0 - 98 (inclusive) by 2s.
		Range range(500, true);		
		GetResult result = buf.GetRows(range);		

		//50 different values..
		CFIX_ASSERT(result.Data.size() == 50);

		int expectedNum = 0;
		for (int i = 0; i < result.Data.size(); i++)
		{
			auto item = result.Data[i];
			//Value should be our expected number;
			CFIX_ASSERT(expectedNum == *(int*)item.first);

			//should be one id per value (unique buffer)
			CFIX_ASSERT(item.second.size() == 1);

			//id should be expected number
			CFIX_ASSERT(expectedNum == *(int*)item.second[0]);

			expectedNum += 2;
		}

		//We should end up on 100 if we've iterated all the values
		CFIX_ASSERT(expectedNum == 100);


		//Range: Entire set descending
		//Expected result: values 98 - 0 (inclusive) by 2s.
		range = Range(500, false);
		result = buf.GetRows(range);

		//50 different values..
		CFIX_ASSERT(result.Data.size() == 50);

		expectedNum = 98;
		for (int i = 0; i < result.Data.size(); i++)
		{
			auto item = result.Data[i];
			//Value should be our expected number;
			CFIX_ASSERT(expectedNum == *(int*)item.first);

			//should be one id per value (unique buffer)
			CFIX_ASSERT(item.second.size() == 1);

			//id should be expected number
			CFIX_ASSERT(expectedNum == *(int*)item.second[0]);

			expectedNum -= 2;
		}

		//We should end up on 100 if we've iterated all the values
		CFIX_ASSERT(expectedNum == -2);


		//Range: 0 (exclusive) - end (inclusive) ascending
		//Expected result: values 2 - 98 (inclusive)
		int startValue = 0;
		RangeBound startBound(&startValue, false);

		range = Range(500, true, startBound);

		result = buf.GetRows(range);		

		//50 different values..
		CFIX_ASSERT(result.Data.size() == 49);

		expectedNum = 2;
		for (int i = 0; i < result.Data.size(); i++)
		{
			auto item = result.Data[i];
			//Value should be our expected number;
			CFIX_ASSERT(expectedNum == *(int*)item.first);

			expectedNum += 2;
		}

		//We should end up on 100 if we've iterated all the values
		CFIX_ASSERT(expectedNum == 100);



	}
};

CFIXCC_BEGIN_CLASS( UniqueBufferTest )
	CFIXCC_METHOD( RangeTests )
CFIXCC_END_CLASS()