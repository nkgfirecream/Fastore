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

int StringCompare(const void* left, const void* right)
{
	return ((fs::wstring*)left)->compare(*(fs::wstring*)right);
}

fs::wstring StringString(const void* item)
{
	return *(fs::wstring*)item;
}

size_t  StringHash(const void* item)
{
	static eastl::string_hash<fs::wstring> hash;
	return hash(*(fs::wstring*)item);
}

ScalarType GetStringType()
{
	ScalarType type;
	type.Compare = StringCompare;
	type.Free = NULL;
	type.Size = sizeof(fs::wstring*);
	type.ToString = StringString;
	type.Hash = StringHash;
	return type;
}

// PString type

int PStringCompare(const void* left, const void* right)
{
	return (*(fs::wstring**)left)->compare(**(fs::wstring**)right);
}

fs::wstring PStringString(const void* item)
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

int LongCompare(const void* left, const void* right)
{
	return (long)left - (long)right;
}

fs::wstring LongString(const void* item)
{
	wstringstream result;
	result << (long)item;
	return result.str();
}

size_t LongHash(const void* item)
{
	static eastl::hash<long> hash;
	return hash((long)item);
}

ScalarType GetLongType()
{
	ScalarType type;
	type.Compare = LongCompare;
	type.Free = NULL;
	type.Size = sizeof(long);
	type.ToString = LongString;
	type.Hash = LongHash;
	return type;
}

// Int ScalarType

int IntCompare(const void* left, const void* right)
{
	return *(int*)left - *(int*)right;
}

fs::wstring IntString(const void* item)
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

int PLongCompare(const void* left, const void* right)
{
	return *(long*)left - *(long*)right;
}

fs::wstring PLongString(const void* item)
{
	wstringstream mystream;
    mystream << *(long*)item;
	return mystream.str();
}

size_t PLongHash(const void* item)
{
	static eastl::hash<long> hash;
	return hash(*(long*)item);
}

ScalarType GetPLongType()
{
	ScalarType type;
	type.Compare = PLongCompare;
	type.Free = IndirectDelete;
	type.Size = sizeof(long*);
	type.ToString = PLongString;
	type.Hash = PLongHash;
	return type;
}

