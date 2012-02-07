#include "typedefs.h"
#include <conio.h>
#include <tbb\queuing_mutex.h>
#include <iostream>

#include "BTree.h"
#include "Util/Stopwatch.h"
#include "Column/ColumnHash.h"
#include "Schema/standardtypes.h"
#include "KeyTree.h"


using namespace std;

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
	cout << "Testing Random Strings...";
	
	long numrows = 1000000;

	BTree tree(GetStringType(), GetStringType());	
	BTree tree2(GetPStringType(), GetPStringType());

	Stopwatch* watch = new Stopwatch();
	Stopwatch* watch2 = new Stopwatch();

	Leaf* dummy;
	Leaf* dummy2;
	for(int i = 0; i < numrows; i++)
	{
		fs::wstring insert = RandomString(rand() % 8 + 1);	
		watch->StartTimer();
		tree.Insert(&insert, &insert, &dummy);	
		watch->StopTimer();

		fs::wstring* insert2 = new fs::wstring(insert);
		watch2->StartTimer();
		tree2.Insert(&insert2, &insert2, &dummy2);
		watch2->StopTimer();
	}

	double secs = watch->TotalTime();
	cout << " secs: " << secs << "\r\n";	
	cout << "Rows per second WString: " << numrows / secs << "\r\n";

	double secs2 = watch2->TotalTime();
	cout << " secs: " << secs2 << "\r\n";	
	cout << "Rows per second WString*: " << numrows / secs2 << "\r\n";
}

void SequentialPLongTest()
{
	cout << "Testing Sequential PLongs...";
	
	long numrows = 10000000;

	BTree tree(GetPLongType(), GetPLongType());	

	Stopwatch watch;

	Leaf* dummy;
	for(int i = 0; i < numrows; i++)
	{
		long* item = new long;

		*item = i;

		watch.StartTimer();
		tree.Insert(&item, &item, &dummy);	
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "Rows per second: " << numrows / secs << "\r\n";

	//wcout << tree->ToString();
}

void SequentialLongTest()
{
	cout << "Testing Sequential Longs...";
	
	long numrows = 10000000;

	BTree tree(GetLongType(), GetLongType());	

	Stopwatch watch;

	Leaf* dummy;
	for(long i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		tree.Insert(&i, &i, &dummy);	
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "Rows per second: " << numrows / secs << "\r\n";

	//wcout << tree->ToString();
}

void SequentialIntTest()
{
	cout << "Testing Sequential Ints...";
	
	long numrows = 1000000;

	BTree tree(GetIntType(), GetIntType());	

	Stopwatch watch;

	Leaf* dummy;
	for(int i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		tree.Insert(&i, &i, &dummy);	
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "Rows per second: " << numrows / secs << "\r\n";

	//wcout << tree->ToString();
}

void GuidTest()
{
	//To be implemented
}

void QueueingMutexTest()
{
	tbb::queuing_mutex qm;

	cout << "Testing Queueing mutex...";
	Stopwatch watch;
	long numrows = 100000000;
	
	unsigned int val = 0;
	for(int i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		tbb::queuing_mutex::scoped_lock lock(qm);
		lock.release();
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	cout << "Mutexes per second: " << numrows / secs << "\r\n";
}

void InterlockedTest()
{
	cout << "Testing InterlockSpeed...";
	Stopwatch watch;
	long numrows = 100000000;
	
	unsigned int val = 0;
	for(int i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		while(InterlockedCompareExchange(&val, 1, 0) == 1)	
		InterlockedDecrement(&val);		
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	cout << "Interlocks per second: " << numrows / secs << "\r\n";
}

void ArrayCopyTest()
{
	int numrows = 10000000;

	int* intarray = (int*)alloca(numrows);

	Stopwatch watch;

	cout << "Testing Assignment speed...";

	for(int i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		intarray[i] = i;
		watch.StopTimer();
	}

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	cout << "Assignments per second: " << numrows / secs << "\r\n";

	watch.Reset();
	cout << "Testing Copy speed...";
	for(int i = 0; i < numrows; i++)
	{
		watch.StartTimer();
		memcpy(&intarray[i], &i, sizeof(int));
		watch.StopTimer();
	}

	secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	cout << "Copies per second: " << numrows / secs << "\r\n";
}

void BTreeIteratorTest()
{
	BTree tree(GetLongType(),GetLongType());
	long numrows = 1000;
	Leaf* dummy;
	long i = 0;
	for(i = 0; i < numrows; i++)
	{
		tree.Insert(&i,&i,&dummy);
	}

	BTree::iterator start = tree.begin();

	i = 0;
	Stopwatch watch;
	watch.GetFrequency();
 	while(!start.End())
	{		
		cout << *(long*)(*start).first << "\n\r";
		watch.StartTimer();
		start++;
		watch.StopTimer();
	}

	/*start = tree.begin();
	
	while (start != end)
	{
		cout << *(long*)*end << "\n\r";
		end--;
	}*/

	double secs = watch.TotalTime();
	cout << " secs: " << secs << "\r\n";
	cout << "iterations per second: " << numrows / secs << "\r\n";

}

void ColumnHashTest()
{
	cout << "Testing ColumnHash...";

	ColumnHash* hash = new ColumnHash(GetLongType(), GetLongType());

	long numrows = 100;
	long numIds = 10;
	
	for (long i = 0; i < numrows; i++)
	{
		for (long j = 0; j < numIds; j++)
		{
			hash->Include(&i,&j);
		}
	}

	cout << "Rows inserted";
}

void TestEAHashSet()
{
	eastl::hash_set<void*> set;
	
	long numrows = 10000;
	for (long i = 0; i < numrows; i++)
	{
		set.insert(&i);
	}
}

void main()
{
	//BTreeIteratorTest();
	//QueueingMutexTest();
	//StringTest();
	//SequentialLongTest();
	//SequentialIntTest();
	//SequentialPLongTest();
	//InterlockedTest();
	//ArrayCopyTest();
	//GuidTest();
	//ColumnHashTest();
	TestEAHashSet();
	getch();
}


