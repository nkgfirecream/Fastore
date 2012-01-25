#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <time.h>
#include "btree.h"
#include "Stopwatch.h"
#include <sstream>
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
	if( *(long *)left < *(long *)right)
		return - 1;
	else if( *(long *)left == *(long *)right)
		return 0;
	else
		return 1;
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

	BTree* tree = new BTree(128,128, StringCompare, StringString);	

	Stopwatch *watch = new Stopwatch();

	cout << " freq: " << watch->GetFrequency() << "\r\n";	

	for(int i = 0; i < numrows; i++)
	{
		wchar_t* insert = RandomString(rand() % 8 + 1);	
		watch->StartTimer();
		tree->Insert((void*)insert,(void*)insert);	
		watch->StopTimer();
	}

	double secs = watch->TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "Rows per second: " << numrows / secs;
}

void SequentialLongTest()
{
	cout << "\nTesting  Sequential Longs...";
	
	long numrows = 1000000;

	BTree* tree = new BTree(128,128, LongCompare, LongString);	

	Stopwatch *watch = new Stopwatch();

	cout << " freq: " << watch->GetFrequency() << "\r\n";	

	for(int i = 0; i < numrows; i++)
	{
		long* item = new long;

		*item = i;

		watch->StartTimer();
		tree->Insert(item, item);	
		watch->StopTimer();
	}

	double secs = watch->TotalTime();
	cout << " secs: " << secs << "\r\n";
	
	cout << "Rows per second: " << numrows / secs;

	//wcout << tree->ToString();
}

void GuidTest()
{
	//To be implemented
}


void main()
{
	StringTest();
	SequentialLongTest();
	GuidTest();

	int wait;
	cin >> wait;
}


