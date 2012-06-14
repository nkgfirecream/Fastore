#include "StringTypes.h"
#include <functional>

using namespace std;

template <>
void* GetPointerFromString<std::string>(const std::string& source)
{
	return (void*)&source;
}

template <>
void CopyOutOfArray<std::string>(const void* arrpointer, std::string& destination)
{
	string input = (*(string*)arrpointer);
	destination.assign(input.data(), input.size());
}

int StringCompare(const void* left, const void* right)
{
	return ((std::string*)left)->compare(*(std::string*)right);
}

std::wstring StringString(const void* item)
{
	//wstring ws(((fs::string*)item)->begin(),((fs::string*)item)->end());
	return wstring(L"String -> WString - Broken");
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
	IndexOf = CompareIndexOf<std::string, StringCompare>;
	CopyIn = CopyToArray<std::string>;
	CopyOut = CopyOutOfArray<std::string>;
	GetPointer = GetPointerFromString<std::string>;
	Deallocate = DeallocateString;
};

template <>
void* GetPointerFromString<std::wstring>(const std::string& source)
{
	return (void*)&source;
}

template <>
void CopyOutOfArray<std::wstring>(const void* arrpointer, std::string& destination)
{
	//destination.assign(arrpointer, sizeof(T));
	throw "Wstring Copy out not implemented";
}

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
	IndexOf = CompareIndexOf<std::wstring, WStringCompare>;
	CopyIn = CopyToArray<std::wstring>;
	CopyOut = CopyOutOfArray<std::wstring>;
	GetPointer = GetPointerFromString<std::wstring>;
	Deallocate = DeallocateWString;
};