#include "StdAfx.h"
#include <cfixcc.h>

#include <sstream>
#include <iostream>
#include <hash_set>
#include <algorithm>

using namespace std;

//Just a scratch pad. Nothing important here.
class TestScratchPad : public cfixcc::TestFixture
{
public:

	void Test()
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

		CFIX_ASSERT(i == 0);
	}
};

CFIXCC_BEGIN_CLASS( TestScratchPad )
	CFIXCC_METHOD( Test )
CFIXCC_END_CLASS()