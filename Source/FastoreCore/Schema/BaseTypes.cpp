#include "BaseTypes.h"
#include "..\Util/utilities.h"
#include <sstream>
#include <functional>

using namespace std;

int LongCompare(const void* left, const void* right)
{
	return (*(long long*)left - *(long long*)right);
}

std::wstring LongString(const void* item)
{
	long long temp = *(long long *)item;
	std::wstring converted = to_wstring(temp);
	return converted;
}

LongType::LongType()
{
	Name = "Long";
	Compare = LongCompare;
	Size = sizeof(long long);
	ToString = LongString;
	IndexOf = TargetedIndexOf<long long>;
	CopyIn = CopyToArray<long long>;
	CopyOut = CopyOutOfArray<long long>;
	GetPointer = GetPointerFromString<long long>;
	Deallocate = NoOpDeallocate;
}

int IntCompare(const void* left, const void* right)
{
	return *(int*)left - *(int*)right;
}

std::wstring IntString(const void* item)
{
	wstringstream result;
	result << *(int*)item;
	return result.str();
}

IntType::IntType()
{
	Name = "Int";
	Compare = IntCompare;
	Size = sizeof(int);
	ToString = IntString;
	IndexOf = CompareIndexOf<int, IntCompare>;
	CopyIn = CopyToArray<int>;
	CopyOut = CopyOutOfArray<int>;
	GetPointer = GetPointerFromString<int>;
	Deallocate = NoOpDeallocate;
}

int BoolCompare(const void* left, const void* right)
{
	//Arbitrarily put true after false.
	if (*(bool*)left == *(bool*)right)
	{
		return 0;
	} 
	else if (*(bool*)left)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

std::wstring BoolString(const void* item)
{
	wstringstream result;
	result << *(bool*)item;
	return result.str();
}

size_t BoolHash(const void* item)
{
	static std::hash<bool> hash;
	return hash(*(bool*)item);
}

BoolType::BoolType()
{
	Name = "Bool";
	Compare = BoolCompare;
	Size = sizeof(bool);
	ToString = BoolString;
	IndexOf = CompareIndexOf<bool, BoolCompare>;
	CopyIn = CopyToArray<bool>;
	CopyOut = CopyOutOfArray<bool>;
	GetPointer = GetPointerFromString<bool>;
	Deallocate = NoOpDeallocate;
}
