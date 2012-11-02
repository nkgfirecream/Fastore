#include "stdafx.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <cassert>
#include <sstream>
#include <iostream>
#include <hash_set>
#include <algorithm>

#include "RangeTests.h"

#include "..\FastoreCommon\Buffer\TreeBuffer.h"
#include "..\FastoreCommon\Buffer\TreeInlineBuffer.h"
#include "..\FastoreCommon\Buffer\MultiBimapBuffer.h"

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
	
	//Tree buffers should work for both one-one tests and one-many tests. Include/Exclude should enforce that behavior.
	TEST_METHOD(TreeBufferRangeTests)
	{
		//Tree buffer -- has multiple values per key.
		TreeBuffer* buf = new TreeBuffer(standardtypes::Long, standardtypes::Long);
		OneToOneRangeTest(buf);
		delete buf;

		//TODO: Only partially implemented.
		buf = new TreeBuffer(standardtypes::Long, standardtypes::Long);
		OneToManyRangeTest(buf);
		delete buf;
	 }

	TEST_METHOD(TreeInlineBufferRangeTests)
	{
		//Tree buffer -- has multiple values per key.
		TreeInlineBuffer* buf = new TreeInlineBuffer(standardtypes::Long, standardtypes::Long);
		OneToOneRangeTest(buf);
		delete buf;

		//TODO: Only partially implemented.
		buf = new TreeInlineBuffer(standardtypes::Long, standardtypes::Long);
		OneToManyRangeTest(buf);
		delete buf;
	 }

	TEST_METHOD(MultiBimapBufferRangeTests)
	{
		//Tree buffer -- has multiple values per key.
		IColumnBuffer* buf = new MultiBimapBuffer<int64_t>();
		OneToOneRangeTest(buf);
		delete buf;

		//TODO: Only partially implemented.
		buf = new  MultiBimapBuffer<int64_t>();
		OneToManyRangeTest(buf);
		delete buf;
	 }

	//TEST_METHOD(TreeBufferIncludeExclude)
	//{
	//	throw "Not yet implemented";
	//	//TreeBuffer buf(standardtypes::Int, standardtypes::Int);

	//	//bool result;
	//	//int v = 0;
	//	//int k = 0;

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