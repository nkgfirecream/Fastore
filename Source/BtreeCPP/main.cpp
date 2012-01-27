#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <sstream>
#include <conio.h>
#include "BTree.h"
#include "Stopwatch.h"
#include "ColumnHash.h"
using namespace std;

int StringCompare(void* left, void* right)
{
	return wcscmp((const wchar_t*)left, (const wchar_t*)right);
}


wstring StringString(void* item)
{
	return (wchar_t*)item;
}

int LongCompare(void* left, void* right)
{
	return 
		( *(long *)left < *(long *)right) ? -1
			: ( *(long *)left > *(long *)right) ? 1
			: 0;
}

wstring LongString(void* item)
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

	BTree* tree = new BTree(128, 128, StringCompare, StringString);	

	Stopwatch *watch = new Stopwatch();

	cout << " freq: " << watch->GetFrequency() << "\r\n";	

	Leaf* dummy;
	for(int i = 0; i < numrows; i++)
	{
		wchar_t* insert = RandomString(rand() % 8 + 1);	
		watch->StartTimer();
		tree->Insert((void*)insert,(void*)insert, dummy);	
		watch->StopTimer();
	}

	double secs = watch->TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "Rows per second: " << numrows / secs << "\r\n";
}

void SequentialLongTest()
{
	cout << "Testing Sequential Longs...";
	
	long numrows = 10000000;

	BTree* tree = new BTree(128, 128, LongCompare, LongString);	

	Stopwatch* watch = new Stopwatch();

	cout << " freq: " << watch->GetFrequency() << "\r\n";	

	Leaf* dummy;
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

void GuidTest()
{
	//To be implemented
}

void ColumnHashTest()
{
		cout << "Testing ColumnHash";

	ColumnHash* hash = new ColumnHash(StringCompare,StringString);
	long numrows = 1000000;
	Stopwatch* watch = new Stopwatch();
	for(long i = 0; i < numrows; i++)
	{
		wchar_t* insert = RandomString(rand() % 8 + 1);	
		watch->StartTimer();
		hash->Insert(i,insert);
		watch->StopTimer();
	}

	double secs = watch->TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "Rows per second: " << numrows / secs << "\r\n";

}


void main()
{
	StringTest();
	SequentialLongTest();
	GuidTest();
	ColumnHashTest();
	getch();
}


