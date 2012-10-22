#pragma once
#include "stdafx.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "Utilities.h"

template <typename Type> void TestRange(IColumnBuffer* buf, RangeRequest range, Type expectedStart, Type expectedEnd, int expectedValuesCount, int increment, bool expectBOF, bool expectEOF, bool expectLimited)
{
	RangeResult result = buf->GetRows(range);
		
	//Right number of values...
	Assert::AreEqual<size_t>(result.valueRowsList.size(), expectedValuesCount);

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
	Assert::AreEqual<Type>(expectedNum, expectedEnd + (result.valueRowsList.size() > 0 ? increment : 0));
	Assert::IsTrue(result.bof == expectBOF);
	Assert::IsTrue(result.eof == expectEOF);
	Assert::IsTrue(result.limited == expectLimited);
}

template <typename Type> void TestRangeMultiValue(IColumnBuffer* buf, RangeRequest range, Type expectedValueStart, Type expectedValueEnd, int expectedValueCount, int valueIncrement, int rowIdIncrement, int expectedTotalRowIds, bool expectBOF, bool expectEOF, bool expectLimited)
{
	RangeResult result = buf->GetRows(range);

	Type expectedNum = expectedValueStart;
	int expectedTotal = 0;
	for (int i = 0; i < result.valueRowsList.size(); i++)
	{
		//Item is a value-rows  A - 1, 2, 3
		auto item = result.valueRowsList[i];

		Type value = *(Type*)item.value.data();

		//Value should be our expected number;
		Assert::AreEqual<Type>(expectedNum, value);

		for(int j = 0; j < item.rowIDs.size(); j++)
		{
			//each id should be expected number
			Assert::AreEqual<Type>(expectedNum, *(Type*)item.rowIDs[j].data());

			expectedNum += rowIdIncrement;
			expectedTotal++;
		}			
	}

	//We should see the expectedEnd + increment if we've iterated all values, or just expected end if we didn't iterate.
	Assert::AreEqual<Type>(expectedNum, expectedValueEnd + (result.valueRowsList.size() > 0 ? valueIncrement : 0));	
	//Right number of values
	Assert::AreEqual<size_t>(result.valueRowsList.size(), expectedValueCount);
	//Right number of total values & rowIds
	Assert::AreEqual<int>(expectedTotal, expectedTotalRowIds);

	Assert::IsTrue(result.bof == expectBOF);
	Assert::IsTrue(result.eof == expectEOF);
	Assert::IsTrue(result.limited == expectLimited);
}


void OneToOneRangeTest(IColumnBuffer* buf);
void OneToManyRangeTest(IColumnBuffer* buf);