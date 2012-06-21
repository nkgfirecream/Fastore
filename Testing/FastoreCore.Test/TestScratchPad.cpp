#include "stdafx.h"
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <vector>

using namespace std;

//Just a scratch pad. Nothing important here.
TEST_CLASS(TestScratchPad)
{
public:

	TEST_METHOD(Test)
	{
		std::vector<int> vec;

		for (int i = 0; i < 10; i++)
		{
			vec.push_back(i);
		}

		auto begin = vec.begin();
		auto end = vec.end();

		int i = 1000;
		while(begin != end)
		{
			end--;
			i = *end;
		}

		Assert::AreEqual<int>(i, 0);
	}
};