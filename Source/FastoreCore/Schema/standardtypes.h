#pragma once

#include <sstream>
#include <string.h>
#include "Schema/type.h"

using namespace std;

void IndirectDelete(void* item)
{
	delete *(void**)item;
}

// String type

int StringCompare(void* left, void* right)
{
	return ((std::wstring*)left)->compare(*(std::wstring*)right);
}

std::wstring StringString(void* item)
{
	return *(std::wstring*)item;
}

type GetStringType()
{
	type type;
	type.Compare = StringCompare;
	type.Free = NULL;
	type.Size = sizeof(std::wstring);
	type.ToString = StringString;
	return type;
}

// PString type

int PStringCompare(void* left, void* right)
{
	return (*(std::wstring**)left)->compare(**(std::wstring**)right);
}

std::wstring PStringString(void* item)
{
	return **(std::wstring**)item;
}

type GetPStringType()
{
	type type;
	type.Compare = PStringCompare;
	type.Free = IndirectDelete;
	type.Size = sizeof(std::wstring*);
	type.ToString = PStringString;
	return type;
}

// Long Type

int LongCompare(void* left, void* right)
{
	return *(long*)left - *(long*)right;
}

std::wstring LongString(void* item)
{
	wstringstream result;
	result << *(long*)item;
	return result.str();
}

type GetLongType()
{
	type type;
	type.Compare = LongCompare;
	type.Free = NULL;
	type.Size = sizeof(long);
	type.ToString = LongString;
	return type;
}

// Int Type

int IntCompare(void* left, void* right)
{
	return *(int*)left - *(int*)right;
}

std::wstring IntString(void* item)
{
	wstringstream result;
	result << *(int*)item;
	return result.str();
}

type GetIntType()
{
	type type;
	type.Compare = IntCompare;
	type.Free = NULL;
	type.Size = sizeof(int);
	type.ToString = IntString;
	return type;
}

// PLong type

int PLongCompare(void* left, void* right)
{
	return *(long*)left - *(long*)right;
}

std::wstring PLongString(void* item)
{
	wstringstream mystream;
    mystream << **(long**)item;
	return mystream.str();
}

type GetPLongType()
{
	type type;
	type.Compare = PLongCompare;
	type.Free = IndirectDelete;
	type.Size = sizeof(long*);
	type.ToString = PLongString;
	return type;
}
