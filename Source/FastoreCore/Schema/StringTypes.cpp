#include "StringTypes.h"

using namespace std;

int StringCompare(const void* left, const void* right)
{
	return ((fs::string*)left)->compare(*(fs::string*)right);
}

fs::wstring StringString(const void* item)
{
	//wstring ws(((fs::string*)item)->begin(),((fs::string*)item)->end());
	return wstring(L"String -> WString - Broken");
}

size_t StringHash(const void* item)
{
	static std::hash<fs::string> hash;
	return hash(*(fs::string*)item);
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
	Size = sizeof(fs::string);
	ToString = StringString;
	Hash = StringHash;
	IndexOf = CompareIndexOf<fs::string, StringCompare>;
	CopyIn = CopyToArray<fs::string>;
	Encode = EncodeString;
	Decode = DecodeString;
	Allocate = AllocateString;
	Deallocate = DeallocateString;
};

int WStringCompare(const void* left, const void* right)
{
	return ((fs::wstring*)left)->compare(*(fs::wstring*)right);
}

fs::wstring WStringString(const void* item)
{
	return *(fs::wstring*)item;
}

size_t WStringHash(const void* item)
{
	static std::hash<fs::wstring> hash;
	return hash(*(fs::wstring*)item);
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
	Size = sizeof(fs::wstring);
	ToString = WStringString;
	Hash = WStringHash;
	IndexOf = CompareIndexOf<fs::wstring, WStringCompare>;
	CopyIn = CopyToArray<fs::wstring>;
	Encode = EncodeWString;
	Decode = DecodeWString;
	Allocate = AllocateWString;
	Deallocate = DeallocateWString;
};