#pragma once



#include "scalar.h"
#include "..\typedefs.h"
#include <sstream>
#include "EASTL\functional.h"

using namespace std;

void IndirectDelete(void* item)
{
	delete *(void**)item;
}

// String type

int StringCompare(void* left, void* right)
{
	return ((fs::wstring*)left)->compare(*(fs::wstring*)right);
}

fs::wstring StringString(void* item)
{
	return *(fs::wstring*)item;
}

ScalarType GetStringType()
{
	ScalarType type;
	type.Compare = StringCompare;
	type.Free = NULL;
	type.Size = sizeof(std::wstring);
	type.ToString = StringString;
	return type;
}

// PString type

int PStringCompare(void* left, void* right)
{
	return (*(fs::wstring**)left)->compare(**(fs::wstring**)right);
}

fs::wstring PStringString(void* item)
{
	return **(fs::wstring**)item;
}

ScalarType GetPStringType()
{
	ScalarType type;
	type.Compare = PStringCompare;
	type.Free = IndirectDelete;
	type.Size = sizeof(fs::wstring*);
	type.ToString = PStringString;
	return type;
}

// Long ScalarType

int LongCompare(void* left, void* right)
{
	return (long)left - (long)right;
}

fs::wstring LongString(void* item)
{
	wstringstream result;
	result << (long)item;
	return result.str();
}

ScalarType GetLongType()
{
	ScalarType type;
	type.Compare = LongCompare;
	type.Free = NULL;
	type.Size = sizeof(long);
	type.ToString = LongString;

	type.HashCompare = [type](void* left, void* right)->bool
	{
		return type.Compare(left,right) < 0;
	};

	eastl::hash<long> hash;

	type.Hash = [hash](void* item)->size_t
	{
		return hash((long)item);
	};
	return type;
}

// Int ScalarType

int IntCompare(void* left, void* right)
{
	return *(int*)left - *(int*)right;
}

fs::wstring IntString(void* item)
{
	wstringstream result;
	result << *(int*)item;
	return result.str();
}

ScalarType GetIntType()
{
	ScalarType type;
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

fs::wstring PLongString(void* item)
{
	wstringstream mystream;
    mystream << **(long**)item;
	return mystream.str();
}

ScalarType GetPLongType()
{
	ScalarType type;
	type.Compare = PLongCompare;
	type.Free = IndirectDelete;
	type.Size = sizeof(long*);
	type.ToString = PLongString;
	return type;
}

