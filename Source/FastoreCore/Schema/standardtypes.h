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
	
	// String type
	int StringCompare(const void*, const void*);
	fs::wstring StringString(const void*);
	size_t  StringHash(const void*);
	ScalarType GetStringType();

	// Long ScalarType
	int LongCompare(const void*, const void*);
	fs::wstring LongString(const void*);
	size_t LongHash(const void*);
	ScalarType GetLongType();

	// PString type
	int PStringCompare(const void*, const void*);
	fs::wstring PStringString(const void*);
	ScalarType GetPStringType();

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

	// PLong type
	int PLongCompare(const void*, const void*);
	fs::wstring PLongString(const void*);
	size_t PLongHash(const void*);
	ScalarType GetPLongType();


	//HashSetType
	ScalarType GetHashSetType();

	//KeyVectorType
	ScalarType GetKeyVectorType();
}