#include "StringTypes.h"

using namespace std;

int StringCompare(const void* left, const void* right)
{
	return ((std::string*)left)->compare(*(std::string*)right);
}

std::wstring StringString(const void* item)
{
	//wstring ws(((fs::string*)item)->begin(),((fs::string*)item)->end());
	return wstring(L"String -> WString - Broken");
}

size_t StringHash(const void* item)
{
	static std::hash<std::string> hash;
	return hash(*(std::string*)item);
}

void EncodeString(const void* input, std::string& output)
{
	auto in = *(std::string*)input;
	output.assign(in.data(), in.size());
}

void DecodeString(const std::string& input, void* output)
{
	(*(string*)output).assign(input.data(), input.size());
}

void* AllocateString()
{
	return new std::string();
}

void DeallocateString(void* items, const int count)
{
	for (int i = 0; i < count; i++)
		((std::string*)items)[i].~basic_string();
}

StringType::StringType()
{
	Name = "String";
	Compare = StringCompare;
	Size = sizeof(std::string);
	ToString = StringString;
	Hash = StringHash;
	IndexOf = CompareIndexOf<std::string, StringCompare>;
	CopyIn = CopyToArray<std::string>;
	Encode = EncodeString;
	Decode = DecodeString;
	Allocate = AllocateString;
	Deallocate = DeallocateString;
};

int WStringCompare(const void* left, const void* right)
{
	return ((std::wstring*)left)->compare(*(std::wstring*)right);
}

std::wstring WStringString(const void* item)
{
	return *(std::wstring*)item;
}

size_t WStringHash(const void* item)
{
	static std::hash<std::wstring> hash;
	return hash(*(std::wstring*)item);
}

void EncodeWString(const void* input, std::string& output)
{
	// TODO: complete wstring
	//Encoder::WriteWString(*(std::string*)input, output);
	throw "WString implementation incomplete.";
}

void DecodeWString(const std::string& input, void* output)
{
	//Encoder::ReadWString(input, *(string*)output);
	throw "WString implementation incomplete.";
}

void* AllocateWString()
{
	return new std::wstring();
}

void DeallocateWString(void* items, const int count)
{
	for (int i = 0; i < count; i++)
		((std::wstring*)items)[i].~basic_string();
}

WStringType::WStringType()
{
	Name = "WString";
	Compare = WStringCompare;
	Size = sizeof(std::wstring);
	ToString = WStringString;
	Hash = WStringHash;
	IndexOf = CompareIndexOf<std::wstring, WStringCompare>;
	CopyIn = CopyToArray<std::wstring>;
	Encode = EncodeWString;
	Decode = DecodeWString;
	Allocate = AllocateWString;
	Deallocate = DeallocateWString;
};