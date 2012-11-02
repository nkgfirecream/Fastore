#include "BaseTypes.h"
#include "../Utility/utilities.h"
#include <sstream>
#include <functional>
#include <stdint.h>

using namespace std;

int LongCompare(const void* left, const void* right)
{
	   const int64_t result = *reinterpret_cast<const int64_t*>(left) - *reinterpret_cast<const int64_t*>(right);
	   if (result > 0)
		   return 1;
	   else if (result == 0)
		   return 0;
	   else
		   return -1;
}

std::wstring LongString(const void* item)
{
	const int64_t temp = *reinterpret_cast<const int64_t*>(item) ;
	wostringstream os;
	os << temp;
	return os.str();
}

LongType::LongType()
{
	Name = "Long";
	Compare = LongCompare;
	Size = sizeof(int64_t);
	ToString = LongString;
	IndexOf =  CompareIndexOf<int64_t, LongCompare>;
	CopyIn = CopyToArray<int64_t>;
	CopyOut = CopyOutOfArray<int64_t>;
	GetPointer = GetPointerFromString<int64_t>;
	Deallocate = NoOpDeallocate;
}

int IntCompare(const void* left, const void* right)
{
	return *reinterpret_cast<const int*>(left) - *reinterpret_cast<const int*>(right);
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
		return 0;
	else if (*(bool*)left)
		return 1;
	else
		return -1;
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


int DoubleCompare(const void* left, const void* right)
{
	//TODO: Make this configurable
	double e = .000000000000005;
	double result =  *(double*)left - *(double*)right;
	if (result > e)
		return 1;
	else if (result < -e)
		return -1;
	else
		return 0;
}

std::wstring DoubleString(const void* item)
{
	const double temp = *reinterpret_cast<const double*>(item) ;
	wostringstream os;
	os << temp;
	return os.str();
}

DoubleType::DoubleType()
{
	Name = "Double";
	Compare = DoubleCompare;
	Size = sizeof(double);
	ToString = DoubleString;
	IndexOf =  CompareIndexOf<double, DoubleCompare>;
	CopyIn = CopyToArray<double>;
	CopyOut = CopyOutOfArray<double>;
	GetPointer = GetPointerFromString<double>;
	Deallocate = NoOpDeallocate;
}
