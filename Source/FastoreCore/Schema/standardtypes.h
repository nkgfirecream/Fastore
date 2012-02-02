#pragma once

#include <sstream>
#include <string.h>
#include "Schema\type.h"
#include "Schema\typedefs.h"

using namespace std;

void IndirectDelete(void* item)
{
	delete *(void**)item;
}

// String type

int StringCompare(void* left, void* right)
{
	return ((fstring*)left)->compare(*(fstring*)right);
}

fstring StringString(void* item)
{
	return *(fstring*)item;
}

Type GetStringType()
{
	Type type;
	type.Compare = StringCompare;
	type.Free = NULL;
	type.Size = sizeof(std::wstring);
	type.ToString = StringString;
	return type;
}

// PString type

int PStringCompare(void* left, void* right)
{
	return (*(fstring**)left)->compare(**(fstring**)right);
}

fstring PStringString(void* item)
{
	return **(fstring**)item;
}

Type GetPStringType()
{
	Type type;
	type.Compare = PStringCompare;
	type.Free = IndirectDelete;
	type.Size = sizeof(fstring*);
	type.ToString = PStringString;
	return type;
}

// Long Type

int LongCompare(void* left, void* right)
{
	return *(long*)left - *(long*)right;
}

fstring LongString(void* item)
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

fstring IntString(void* item)
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

fstring PLongString(void* item)
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

// HashSet type -- Can't be used as a keytype.
Type GetHashSetType()
{
	Type type;
	type.Compare = NULL;
	type.Free = IndirectDelete;
	type.Size = sizeof(ColumnHashSet*);
	type.ToString = NULL;
	return type;
}
