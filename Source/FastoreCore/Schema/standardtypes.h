#pragma once

#include "scalar.h"
#include "..\typedefs.h"

using namespace std;

namespace standardtypes
{
	//Delete Functions
	void IndirectDelete(void* item);

	//IndexOf Functions
	template <typename T>
	int NumericIndexOf(const char* items, const int count, void *key);
	template <typename T>
	int ScaledIndexOf(const char* items, const int count, void *key);
	template <typename T, ScalarType::CompareFunc Comparer>
	int CompareIndexOf(const char* items, const int count, void *key);
	template <typename T>
	int TargetedIndexOf(const char* items, const int count, void *key);
	template <typename T>
	void CopyToArray(const void* item, void* arrpointer);
	template <typename T>
	void Encode(const void* input, std::string& output);
	template <typename T>
	void Decode(const std::string& input, void*& output);
	template <typename T>
	void* Allocate();
	
	// WString type
	int WStringCompare(const void*, const void*);
	fs::wstring WStringString(const void*);
	size_t  WStringHash(const void*);
	ScalarType GetWStringType();

	// Long ScalarType
	int LongCompare(const void*, const void*);
	fs::wstring LongString(const void*);
	size_t LongHash(const void*);
	ScalarType GetLongType();

	// Int ScalarType
	int IntCompare(const void*, const void*);
	fs::wstring IntString(const void*);
	size_t IntHash(const void*);
	ScalarType GetIntType();

	//Bool ScalarType
	int BoolCompare(const void*, const void*);
	fs::wstring BoolString(const void*);
	size_t BoolHash(const void*);
	ScalarType GetBoolType();

	// String type
	int StringCompare(const void*, const void*);
	fs::wstring StringString(const void*);
	size_t  StringHash(const void*);
	ScalarType GetStringType();


	//HashSetType
	ScalarType GetHashSetType();

	//KeyVectorType
	ScalarType GetKeyVectorType();
}