#include "stdafx.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <sstream>
#include <iostream>
#include <hash_set>
#include <algorithm>

#include "RangeTests.h"
#include "Column\UniqueBuffer.h"
#include "Column\UniqueInlineBuffer.h"

using namespace std;


TEST_CLASS(UniqueBufferTest)
{
public:

	//Unique buffers should only work for one-one tests. The Include/Exclude test should enforce that behavior.
	TEST_METHOD(UniqueRangeTests)
	{
		//Unique buffer -- one key has one and only one value
		UniqueBuffer* buf = new UniqueBuffer(standardtypes::Long, standardtypes::Long);
		OneToOneRangeTest(buf);	
		delete buf;
	}

	TEST_METHOD(UniqueInlineRangeTests)
	{
		//Unique buffer -- one key has one and only one value
		UniqueInlineBuffer* buf = new UniqueInlineBuffer(standardtypes::Long, standardtypes::Long);
		OneToOneRangeTest(buf);	
		delete buf;
	}
	

	//TEST_METHOD(UniqueIncludeExclude)
	//{
	//	throw "Not yet implemented";
	//	//UniqueBuffer buf(standardtypes::Int, standardtypes::Int);

	////	bool result;
	////	int v = 0;
	////	int k = 0;

	//	////Non existing inserts should be ok
	//	//result = buf.Include(&v, &k);
	//	//Assert::AreEqual(result == true);

	//	////Duplicate insertions should not insert
	//	//result = buf.Include(&v, &k);
	//	//Assert::AreEqual(result == false);

	//	////Duplicate keys should not insert
	//	//v = 2;
	// //   k = 0;
	//	//result = buf.Include(&v, &k);
	//	//Assert::AreEqual(result == false);

	//	////Duplicate values should not insert
	//	//v = 0;
	// //   k = 2;
	//	//result = buf.Include(&v, &k);
	//	//Assert::AreEqual(result == false);

	//	////End of this should still be zero
	//	//k = 0;
	//	//void* value = buf.GetValue(&k);
	//	//Assert::AreEqual(*(int*)value == 0);

	//	////Values not present should not exclude
	//	//v = 1;
	//	//k = 0;
	//	//result = buf.Exclude(&v, &k);
	//	//Assert::AreEqual(result == false);

	//	////Keys not present should not exclude
	//	//v = 0;
	//	//k = 1;
	//	//result = buf.Exclude(&k);
	//	//Assert::AreEqual(result == false);

	//	//result = buf.Exclude(&v, &k);
	//	//Assert::AreEqual(result == false);

	//	////Keys present should exclude
	//	//v = 0;
	//	//k = 0;
	//	//result = buf.Exclude(&v, &k);
	//	//Assert::AreEqual(result == true);

	//	////A bunch of insertions should work...
	//	//int numrows = 10000;
	//	//for (int i = 0; i <= numrows; i++)
	//	//{
	//	//	result = buf.Include(&i,&i);
	//	//	Assert::AreEqual(result == true);
	//	//}

	//	////A bunch of exclusions should work...
	//	//for (int i = numrows / 2; i <= numrows; i++)
	//	//{
	//	//	result = buf.Exclude(&i,&i);
	//	//	Assert::AreEqual(result == true);
	//	//}

	//	////All the values should still be the same...
	//	//for (int i = 0; i < numrows / 2; i++)
	//	//{
	//	//	value = buf.GetValue(&i);
	//	//	Assert::AreEqual(*(int*)value == i);
	//	//}
	//}	
};