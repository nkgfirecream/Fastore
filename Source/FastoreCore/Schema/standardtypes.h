#pragma once

#include <sstream>
#include <EASTL\string.h>
#include "type.h"

void IndirectDelete(void* item)
{
	delete *(void**)item;
}

// String type

int StringCompare(void* left, void* right)
{
	return ((wstring*)left)->compare(*(wstring*)right);
}

wstring StringString(void* item)
{
	return *(wstring*)item;
}

Type GetStringType()
{
	Type type;
	type.Compare = StringCompare;
	type.Free = NULL;
	type.Size = sizeof(wstring);
	type.ToString = StringString;
	return type;
}

// PString type

int PStringCompare(void* left, void* right)
{
	return (*(wstring**)left)->compare(**(wstring**)right);
}

wstring PStringString(void* item)
{
	return **(wstring**)item;
}

Type GetPStringType()
{
	Type type;
	type.Compare = PStringCompare;
	type.Free = IndirectDelete;
	type.Size = sizeof(wstring*);
	type.ToString = PStringString;
	return type;
}

// Long Type

int LongCompare(void* left, void* right)
{
	return *(long*)left - *(long*)right;
}

wstring LongString(void* item)
{
	wstringstream result;
	result << *(long*)item;
	return result.str();
}

Type GetLongType()
{
	Type type;
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

wstring IntString(void* item)
{
	wstringstream result;
	result << *(int*)item;
	return result.str();
}

Type GetIntType()
{
	Type type;
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

wstring PLongString(void* item)
{
	wstringstream mystream;
    mystream << **(long**)item;
	return mystream.str();
}

Type GetPLongType()
{
	Type type;
	type.Compare = PLongCompare;
	type.Free = IndirectDelete;
	type.Size = sizeof(long*);
	type.ToString = PLongString;
	return type;
}
