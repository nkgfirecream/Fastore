#include "BaseTypes.h"
#include "..\typedefs.h"
#include "..\Encoder.h"
#include "..\Util\utilities.h"
#include <sstream>

using namespace std;

void DecodeLong(const std::string& input, void* output)
{
	// WARNING: Platform specific
	*(long long*)output = *(long long*)input.data();
}

void EncodeLong(const void* input, std::string& output)
{
	// WARNING: Platform specific
	output.assign((char*)input, sizeof(long long));
}

void* AllocateLong()
{
	return new long long();
}

int LongCompare(const void* left, const void* right)
{
	return (*(long long*)left - *(long long*)right);
}

fs::wstring LongString(const void* item)
{
	long long temp = *(long long *)item;
	fs::wstring converted = to_wstring(temp);
	return converted;
}

size_t LongHash(const void* item)
{
	static std::hash<long long> hash;
	return hash(*(long long*)item);
}

LongType::LongType()
{
	Name = "Long";
	Compare = LongCompare;
	Size = sizeof(long long);
	ToString = LongString;
	Hash = LongHash;
	IndexOf = TargetedIndexOf<long long>;
	CopyIn = CopyToArray<long long>;
	Encode = EncodeLong;
	Decode = DecodeLong;
	Allocate = AllocateLong;
	Deallocate = NoOpDeallocate;
}

void* AllocateInt()
{
	return new int();
}

void DecodeInt(const std::string& input, void* output)
{
	// WARNING: Platform specific
	*(int*)output = *(int*)input.data();
}

void EncodeInt(const void* input, std::string& output)
{
	// WARNING: Platform specific
	output.assign((char*)input, sizeof(int));
}

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

size_t IntHash(const void* item)
{
	static std::hash<int> hash;
	return hash(*(int*)item);
}

IntType::IntType()
{
	Name = "Int";
	Compare = IntCompare;
	Size = sizeof(int);
	ToString = IntString;
	IndexOf = CompareIndexOf<int, IntCompare>;
	CopyIn = CopyToArray<int>;
	Hash = IntHash;
	Encode = EncodeInt;
	Decode = DecodeInt;
	Allocate = AllocateInt;
	Deallocate = NoOpDeallocate;
}

void* AllocateBool()
{
	return new bool();
}

void DecodeBool(const std::string& input, void* output)
{
	// WARNING: Platform specific
	*(bool*)output = *(bool*)input.data();
}

void EncodeBool(const void* input, std::string& output)
{
	// WARNING: Platform specific
	output.assign((char*)input, sizeof(bool));
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

fs::wstring BoolString(const void* item)
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
	Hash = BoolHash;
	Encode = EncodeBool;
	Decode = DecodeBool;
	Allocate = AllocateBool;
	Deallocate = NoOpDeallocate;
}
