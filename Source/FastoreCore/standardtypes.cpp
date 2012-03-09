#include "typedefs.h"
#include "Schema\standardtypes.h"
#include <sstream>
#include "EASTL\functional.h"
#include "EASTL\hash_set.h"

using namespace std;

void standardtypes::IndirectDelete(void* item)
{
	delete *(void**)item;
}

template <typename T>
int standardtypes::ScaledIndexOf(const char* items, const int count, void *key)
{
	int lo = 0;
	int hi = count - 1;
	int split = 0;
	int result = -1;
	T loVal;
	T hiVal;
	T val;

	while (lo <= hi)
	{
		//split = (lo + hi)  >> 1;   // EASTL says: We use '>>1' here instead of '/2' because MSVC++ for some reason generates significantly worse code for '/2'. Go figure.
		loVal = ((T*)items)[lo];
		hiVal = ((T*)items)[hi];
		val = *(T*)key;
		if (val <= loVal)
			return val == loVal ? lo : ~lo;
		if (val >= hiVal)
			return val == hiVal ? hi : ~(hi + 1);
		split = lo + (int)((val - (float)loVal) / (hiVal - (float)loVal) * (hi - lo));
			//((val >> 1) - (loVal >> 1)) 
			//	* (hi - lo) 
			//	/ ((hiVal >> 1) - (loVal >> 1)) 
			//	+ lo;
		result = val - ((T*)items)[split];

		if (result == 0)
			return split;
		else if (result < 0)
		{
			hi = split - 1;
			++lo;
		}
		else
		{
			lo = split + 1;
			--hi;
		}
	}

	return ~lo;
}

template <typename T>
int standardtypes::NumericIndexOf(const char* items, const int count, void *key)
{
	int lo = 0;
	int hi = count - 1;
	int split = 0;
	int result = -1;

	while (lo <= hi)
	{
		split = (lo + hi)  >> 1;   // EASTL says: We use '>>1' here instead of '/2' because MSVC++ for some reason generates significantly worse code for '/2'. Go figure.
		result = *(T*)key - ((T*)items)[split];

		if (result == 0)
			return split;
		else if (result < 0)
			hi = split - 1;
		else
			lo = split + 1;
	}

	return ~lo;
}

template <typename T, typename COMP>
int standardtypes::CompareIndexOf(const char* items, const int count, void *key)
{
	int lo = 0;
	int hi = count - 1;
	int split = 0;
	int result = -1;

	while (lo <= hi)
	{
		split = (lo + hi)  >> 1;   // EASTL says: We use '>>1' here instead of '/2' because MSVC++ for some reason generates significantly worse code for '/2'. Go figure.
		result = COMP(*(T*)key, &((T*)items)[split]);

		if (result == 0)
			return split;
		else if (result < 0)
			hi = split - 1;
		else
			lo = split + 1;
	}

	return ~lo;
}

// String type
int standardtypes::StringCompare(const void* left, const void* right)
{
	return ((fs::wstring*)left)->compare(*(fs::wstring*)right);
}

fs::wstring standardtypes::StringString(const void* item)
{
	return *(fs::wstring*)item;
}

size_t  standardtypes::StringHash(const void* item)
{
	static eastl::string_hash<fs::wstring> hash;
	return hash(*(fs::wstring*)item);
}

ScalarType standardtypes::GetStringType()
{
	ScalarType type;
	type.Compare = StringCompare;
	type.Free = NULL;
	type.Size = sizeof(fs::wstring);
	type.ToString = StringString;
	type.Hash = StringHash;
	//type.IndexOf = CompareIndexOf<fs::wstring, StringCompare>;
	return type;
}

// Long ScalarType
int standardtypes::LongCompare(const void* left, const void* right)
{
	return (int)(*(long long*)left - *(long long*)right);
}

fs::wstring standardtypes::LongString(const void* item)
{
	wstringstream result;
	result << *(long long*)item;
	return result.str();
}

size_t standardtypes::LongHash(const void* item)
{
	static eastl::hash<long long> hash;
	return hash(*(long long*)item);
}

ScalarType standardtypes::GetLongType()
{
	ScalarType type;
	type.Compare = LongCompare;
	type.Free = NULL;
	type.Size = sizeof(long long);
	type.ToString = LongString;
	type.Hash = LongHash;
	type.IndexOf = ScaledIndexOf<long long>;
	return type;
}

// PString type
int standardtypes::PStringCompare(const void* left, const void* right)
{
	return (*(fs::wstring**)left)->compare(**(fs::wstring**)right);
}

fs::wstring standardtypes::PStringString(const void* item)
{
	return **(fs::wstring**)item;
}

ScalarType standardtypes::GetPStringType()
{
	ScalarType type;
	type.Compare = PStringCompare;
	type.Free = IndirectDelete;
	type.Size = sizeof(fs::wstring*);
	type.ToString = PStringString;
	//type.IndexOf = CompareIndexOf<fs::wstring*, PStringCompare>;
	return type;
}

// Int ScalarType
int standardtypes::IntCompare(const void* left, const void* right)
{
	return *(int*)left - *(int*)right;
}

fs::wstring standardtypes::IntString(const void* item)
{
	wstringstream result;
	result << *(int*)item;
	return result.str();
}

ScalarType standardtypes::GetIntType()
{
	ScalarType type;
	type.Compare = IntCompare;
	type.Free = NULL;
	type.Size = sizeof(int);
	type.ToString = IntString;
	type.IndexOf = ScaledIndexOf<int>;
	return type;
}

// PLong type
int standardtypes::PLongCompare(const void* left, const void* right)
{
	return *(long*)left - *(long*)right;
}

fs::wstring standardtypes::PLongString(const void* item)
{
	wstringstream mystream;
    mystream << *(long*)item;
	return mystream.str();
}

size_t standardtypes::PLongHash(const void* item)
{
	static eastl::hash<long> hash;
	return hash(*(long*)item);
}

ScalarType standardtypes::GetPLongType()
{
	ScalarType type;
	type.Compare = PLongCompare;
	type.Free = IndirectDelete;
	type.Size = sizeof(long*);
	type.ToString = PLongString;
	type.Hash = PLongHash;
	//type.IndexOf = CompareIndexOf<long*, PLongCompare>;
	return type;
}

//HashSet Type
ScalarType standardtypes::GetHashSetType()
{
	ScalarType type;
	type.Compare = NULL;
	type.Free = IndirectDelete;
	type.Size = sizeof(eastl::hash_set<void*, ScalarType, ScalarType>*);
	type.ToString = NULL;
	return type;
}

ScalarType standardtypes::GetKeyVectorType()
{
	ScalarType type;
	type.Compare = NULL;
	type.Free = IndirectDelete;
	type.Size = sizeof(fs::KeyVector*);
	type.ToString = NULL;
	return type;
}


