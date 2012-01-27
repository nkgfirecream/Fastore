#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <sstream>
#include <conio.h>
#include "BTree.h"
#include "Stopwatch.h"
//#include "ColumnHash.h"
using namespace std;

int StringCompare(wstring left, wstring right)
{
	return left.compare(right);
}

wstring StringString(wstring item)
{
	return item;
}

int LongCompare(long left, long right)
{
	return left < right ? -1
		: right < left ? 1
		: 0;
}

wstring LongString(long item)
{
	wstringstream result;
	result << item;
	return result.str();
}

int PLongCompare(void* left, void* right)
{
	return 
		( *(long *)left < *(long *)right) ? -1
			: ( *(long *)left > *(long *)right) ? 1
			: 0;
}

wstring PLongString(void* item)
{
	wstringstream mystream;
    mystream << *(long*)item;
	return mystream.str();
}

wchar_t* RandomString(int length)
{
	char* _chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	wchar_t* result = new wchar_t[length + 1];

	for(int i = 0; i < length; i++)
	{
		int index = rand() % 26;
		result[i] = _chars[index];
	}

	result[length] = '\0';

	return result;
}

void StringTest()
{
	cout << "Testing Random Strings...";
	
	long numrows = 1000000;

	BTree<wstring, wstring>* tree = new BTree<wstring, wstring>(128, 128, StringCompare, StringString, StringString);	

	Stopwatch *watch = new Stopwatch();

	cout << " freq: " << watch->GetFrequency() << "\r\n";	

	Leaf<wstring, wstring>* dummy;
	for(int i = 0; i < numrows; i++)
	{
		wchar_t* insert = RandomString(rand() % 8 + 1);	
		watch->StartTimer();
		tree->Insert(insert, insert, dummy);	
		watch->StopTimer();
	}

	double secs = watch->TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "Rows per second: " << numrows / secs << "\r\n";
}

void SequentialPLongTest()
{
	cout << "Testing Sequential Longs...";
	
	long numrows = 10000000;

	BTree<void*, void*>* tree = new BTree<void*, void*>(128, 128, PLongCompare, PLongString, PLongString);	

	Stopwatch* watch = new Stopwatch();

	cout << " freq: " << watch->GetFrequency() << "\r\n";	

	Leaf<void*, void*>* dummy;
	for(int i = 0; i < numrows; i++)
	{
		long* item = new long;

		*item = i;

		watch->StartTimer();
		tree->Insert(item, item, dummy);	
		watch->StopTimer();
	}

	double secs = watch->TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "Rows per second: " << numrows / secs << "\r\n";

	//wcout << tree->ToString();
}

void SequentialLongTest()
{
	cout << "Testing Sequential Longs...";
	
	long numrows = 10000000;

	BTree<long, long>* tree = new BTree<long, long>(128, 128, LongCompare, LongString, LongString);	

	Stopwatch* watch = new Stopwatch();

	cout << " freq: " << watch->GetFrequency() << "\r\n";	

	Leaf<long, long>* dummy;
	for(int i = 0; i < numrows; i++)
	{
		watch->StartTimer();
		tree->Insert(i, i, dummy);	
		watch->StopTimer();
	}

	double secs = watch->TotalTime();
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
//	ColumnHash* hash = new ColumnHash(StringCompare,StringString);
//	long numrows = 100000;
//	Stopwatch* watch = new Stopwatch();
//	cout << " freq: " << watch->GetFrequency() << "\r\n";	
//	for(long i = 0; i < numrows; i++)
//	{
//		wchar_t* insert = RandomString(rand() % 8 + 1);	
//		watch->StartTimer();
//		hash->Insert(i,insert);
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


void main()
{
	StringTest();
	SequentialLongTest();
	SequentialPLongTest();
	GuidTest();
	//ColumnHashTest();
	getch();
}


