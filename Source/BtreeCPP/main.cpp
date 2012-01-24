#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include "btree.h"
using namespace std;

int StringCompare(void* left, void* right)
{
	return strcmp((const char *)left, (const char *)right);
}

char* StringString(void* item)
{
	return (char*)item;
}

char* RandomString(int length)
{
	string _chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	char* result = new char[length + 1];

	for(int i = 0; i < length; i++)
	{
		int index = rand() % 26;
		result[i] = _chars[index];
	}

	result[length] = '\0';

	return result;
}

void main()
{
	cout << "Testing...";

	long ctr1 = 0, ctr2 = 0, freq = 0;
	long numrows = 1000000;

	long total = 0;

	BTree* tree = new BTree(128,128, StringCompare, StringString);	

	QueryPerformanceCounter((LARGE_INTEGER *)&ctr1);

	for(int i = 0; i < numrows; i++)
	{
		char* insert = RandomString(rand() % 8 + 1);		
		tree->Insert((void*)insert,(void*)insert);		
	}

	QueryPerformanceCounter((LARGE_INTEGER *)&ctr2);

	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

	cout << "Rows per second: " << (numrows /(((ctr2 - ctr1) * 1.0) / freq));
	//cout << tree->ToString();
	int what;
	cin >> what;
}