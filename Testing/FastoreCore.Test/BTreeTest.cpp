#include "StdAfx.h"
#include <cfixcc.h>

#include <sstream>
#include <iostream>

using namespace std;


class BTreeTest : public cfixcc::TestFixture
{
public:
	wstring RandomString(int length)
	{
		const wchar_t* _chars = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

		wstringstream result;

		for (int i = 0; i < length; i++)
		{
			int index = rand() % 26;
			result << _chars[index];
		}
								 
		return result.str();
	}

	void StringTest()
	{

	}

};

CFIXCC_BEGIN_CLASS( BTreeTest )
	CFIXCC_METHOD( StringTest )
CFIXCC_END_CLASS()