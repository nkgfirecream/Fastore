#include "StdAfx.h"
#include "cfixcc.h"
#include "BTree.h"

#include "Schema\standardtypes.h"
#include "Table\table.h"
#include "Column\HashBuffer.h"
#include "Column\UniqueBuffer.h"
//#include "Column\TreeBuffer.h"
#include "KeyTree.h"
#include <conio.h>
#include <tbb\queuing_mutex.h>
#include <iostream>
#include "Util/Stopwatch.h"
#include <sstream>
#include <bitset>
#include "TransactionID.h"
#include "Change.h"
#include "Topology.h"
#include "HostFactory.h"
#include "Database.h"
#include "Session.h"

#include <iostream>
#include <fstream>



using namespace std;
using namespace standardtypes;

class BTreeTest : public cfixcc::TestFixture
{
public:
	fs::wstring RandomString(int length)
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
		//cout << "Testing Random Strings...";
	
		long numrows = 1000000;

		BTree tree(GetStringType(), GetStringType());	

		//Stopwatch* watch = new Stopwatch();
		//Stopwatch* watch2 = new Stopwatch();

		for(int i = 0; i < numrows; i++)
		{
			fs::wstring insert = RandomString(rand() % 8 + 1);	
			//watch->StartTimer();
			BTree::Path path = tree.GetPath(&insert);
			tree.Insert(path, &insert, &insert);	
			//watch->StopTimer();

			fs::wstring* insert2 = new fs::wstring(insert);
			//watch2->StartTimer();
			//BTree::Path path2 = tree.GetPath(&insert2);
			//tree2.Insert(path2, &insert2, &insert2);
			//watch2->StopTimer();
		}

		//double secs = watch->TotalTime();
		//cout << " secs: " << secs << "\r\n";	
		//cout << "\tRows per second WString: " << numrows / secs << "\r\n";

		//double secs2 = watch2->TotalTime();
		//cout << " secs: " << secs2 << "\r\n";	
		//cout << "\tRows per second WString*: " << numrows / secs2 << "\r\n";
	}

};

CFIXCC_BEGIN_CLASS( BTreeTest )
	CFIXCC_METHOD( StringTest )
CFIXCC_END_CLASS()