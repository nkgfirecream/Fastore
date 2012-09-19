#include "stdafx.h"
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "Schema\standardtypes.h"
#include "Column\UniqueBuffer.h"
#include "Serialization.h"
#include "Extensions.h"

#include <stdio.h>

using namespace std;


TEST_CLASS(BufferSerializationTest)
{
public:
	
	TEST_METHOD(SerializeDeserialize)
	{
		//Create blank file
		string filename = "C:\\test.dat";

		//TODO: Update limited behavior to reflect BoF/EoF semantics.
		//Unique buffer -- one key has one and only one value
		UniqueBuffer buf(standardtypes::Int, standardtypes::Int);

		ColumnWrites cw;
		std::vector<Include> includes;

		//Insert values 0 - 9998 (inclusive) in increments of 2 into buffer
		for (int i = 0; i < 10000; i += 2)
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

		Assert::AreEqual<int64_t>(buf.GetStatistic().total, 5000);
		Assert::AreEqual<int64_t>(buf.GetStatistic().unique, 5000);

		//Entire Set
		//Range: Entire set ascending
		//Expected result: values 0 - 98 (inclusive) by 2s.
		RangeRequest range;
		range.__set_limit(500);
		range.__set_ascending(true);
		TestRange(buf, range, 0, 998, 500, 2, true, false, true);

		//Serialize
		BufferSerializer serializer(buf, filename);

		serializer.open();
		while (serializer.writeNextChunk());
		serializer.close();

		//Deserialize
		UniqueBuffer buf2(standardtypes::Int, standardtypes::Int);
		BufferDeserializer deserializer(buf2, filename);

		deserializer.open();
		while (deserializer.readNextChunk());
		deserializer.close();

		//Retest
		TestRange(buf, range, 0, 998, 500, 2, true, false, true);

	}

	void TestRange(UniqueBuffer& buf, RangeRequest range, int expectedStart, int expectedEnd, int expectedValuesCount, int increment, bool expectBOF, bool expectEOF, bool expectLimited)
	{
		RangeResult result = buf.GetRows(range);
		
		//Right number of values...
		Assert::AreEqual<size_t>(result.valueRowsList.size(), expectedValuesCount);

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
		Assert::AreEqual<bool>(result.bof, expectBOF);
		Assert::AreEqual<bool>(result.eof, expectEOF);
		Assert::AreEqual<bool>(result.limited, expectLimited);
	}
};