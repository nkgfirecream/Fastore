#include "stdafx.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "RangeTests.h"
#include "Column\IdentityBuffer.h"

using namespace std;


TEST_CLASS(IdentityBufferTest)
{
public:
	
	//Identity buffers should only work for one-one tests. The Include/Exclude test should enforce that behavior.
	TEST_METHOD(IdentityRangeTests)
	{
		//Identity buffer -- one key has one and only one value. Value is the same type and 'number' as key.
		IdentityBuffer* buf = new IdentityBuffer(standardtypes::Long);
		OneToOneRangeTest(buf);
		delete buf;
	}
};