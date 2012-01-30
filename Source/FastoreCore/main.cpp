#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <sstream>
#include <conio.h>
#include <malloc.h>

#include "BTree.h"
#include "Stopwatch.h"
//#include "ColumnHash.h"
#include "Schema/standardtypes.h"

using namespace std;

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
	cout << "Testing Random Strings...";
	
	long numrows = 1000000;

	BTree tree(GetStringType(), GetStringType());	
	BTree tree2(GetPStringType(), GetPStringType());

	Stopwatch* watch = new Stopwatch();
	Stopwatch* watch2 = new Stopwatch();

	cout << " freq: " << watch->GetFrequency() << "\r\n";	
	cout << " freq2: " << watch2->GetFrequency() << "\r\n";	

	Leaf* dummy;
	Leaf* dummy2;
	for(int i = 0; i < numrows; i++)
	{
		wstring insert = RandomString(rand() % 8 + 1);	
		watch->StartTimer();
		tree.Insert(&insert, &insert, &dummy);	
		watch->StopTimer();

		wstring* insert2 = new wstring(insert);
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

	cout << " freq: " << watch.GetFrequency() << "\r\n";	

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

	cout << " freq: " << watch.GetFrequency() << "\r\n";	

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

	cout << " freq: " << watch.GetFrequency() << "\r\n";	

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

//void ColumnHashTest()
//{
//		cout << "Testing ColumnHash...";
//
//	ColumnHash<long,long>* hash = new ColumnHash<long,long>(LongCompare,LongString,LongString);
//
//	long numrows = 100000;
//	Stopwatch* watch = new Stopwatch();
//	cout << " freq: " << watch->GetFrequency() << "\r\n";	
//	for(long i = 0; i < numrows; i++)
//	{
//		wchar_t* insert = RandomString(rand() % 8 + 1);	
//		watch->StartTimer();
//		hash->Insert(i,i);
//		watch->StopTimer();
//	}
//
//	double secs = watch->TotalTime();
//	cout << " secs: " << secs << "\r\n";
//
//	cout << "Entries per second: " << numrows / secs << "\r\n";
//
//	watch->Reset();
//	
//	for(long i = 0; i < 300; i++)
//	{
//		watch->StartTimer();
//		hash->GetValue(i);
//		watch->StopTimer();
//	}
//
//	secs = watch->TotalTime();
//	cout << " secs: " << secs << "\r\n";
//	cout << "Extractions per second: " << 300 / secs << "\r\n";
//	
//
//}

void InterlockedTest()
{
	cout << "Testing InterlockSpeed...";
	Stopwatch watch;
	long numrows = 100000000;
	cout << " freq: " << watch.GetFrequency() << "\r\n";	
	
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
	cout << " freq: " << watch.GetFrequency() << "\r\n";	

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


void main()
{
	StringTest();
	//SequentialLongTest();
	//SequentialIntTest();
	//SequentialPLongTest();
	//InterlockedTest();
	//ArrayCopyTest();
	//GuidTest();
	//ColumnHashTest();
	getch();
}


