#include "..\typedefs.h"
#include "standardtypes.h"
#include <sstream>
#include <hash_set>

using namespace std;

void standardtypes::IndirectDelete(void* item)
{
	delete *(void**)item;
}

//TODO: Fill out various encoding schemes. Perhaps an add on library?
template <typename T>
void* Decode(const std::string item)
{
	return new std::string(item);
}

template <typename T>
std::string Encode(const void* item)
{
	return *(std::string*)item;
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
		loVal = ((T*)items)[lo];
		hiVal = ((T*)items)[hi];
		val = *(T*)key;

		// Test for value on or over range bounds
		if (val <= loVal)
			return val == loVal ? lo : ~lo;
		if (val >= hiVal)
			return val == hiVal ? hi : ~(hi + 1);

		// Split proportionately to the value scaling
		split = lo + (int)((val - (double)loVal) / (hiVal - (double)loVal) * (hi - lo));
			//(val - loVal) * (hi - lo) / (hiVal - loVal) + lo;		integer version suffers from overflow problems with subtraction and multiplication

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

inline int SignNum(int value)
{
	return (value > 0) - (value < 0);
}

template <typename T>
int standardtypes::TargetedIndexOf(const char* items, const int count, void *key)
{
	if (count == 0)
		return ~0;

	int hi = count - 1;

	T loVal = ((T*)items)[0];
	T hiVal = ((T*)items)[hi];
	T val = *(T*)key;

	// Test for value on or over range bounds
	if (val <= loVal)
		return val == loVal ? 0 : ~0;
	if (val >= hiVal)
		return val == hiVal ? hi : ~count;

	// Split proportionately to the value scaling
	int pos = (int)((val - (double)loVal) / (hiVal - (double)loVal) * hi);

	int diff = (int)(val - ((T*)items)[pos]);

	if (diff == 0)
		return pos;

	//normalize direction
	int direction = SignNum(diff);

	if (direction > 0)
	{
		while (diff > 0)
		{
			++pos;
			if (pos > hi)
				return ~count;
			diff = (int)(val - ((T*)items)[pos]);
		}
	}
	else
	{
		while (diff < 0)
		{
			--pos;
			if (pos < 0)
				return ~0;
			diff = (int)(val - ((T*)items)[pos]);
		}
	}
	return diff == 0 ? pos : ~pos;
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

template <typename T, ScalarType::CompareFunc Comparer>
int standardtypes::CompareIndexOf(const char* items, const int count, void *key)
{
	int lo = 0;
	int hi = count - 1;
	int split = 0;
	int result = -1;

	while (lo <= hi)
	{
		split = (lo + hi)  >> 1;   // EASTL says: We use '>>1' here instead of '/2' because MSVC++ for some reason generates significantly worse code for '/2'. Go figure.
		result = Comparer((T*)key, &((T*)items)[split]);

		if (result == 0)
			return split;
		else if (result < 0)
			hi = split - 1;
		else
			lo = split + 1;
	}

	return ~lo;
}

template <typename T>
void standardtypes::CopyToArray(const void* item, void* arrpointer)
{
	new (arrpointer) T(*(T*)item);
}

// String type
int standardtypes::WStringCompare(const void* left, const void* right)
{
	return ((fs::wstring*)left)->compare(*(fs::wstring*)right);
}

fs::wstring standardtypes::WStringString(const void* item)
{
	return *(fs::wstring*)item;
}

size_t  standardtypes::WStringHash(const void* item)
{
	static std::hash<fs::wstring> hash;
	return hash(*(fs::wstring*)item);
}

ScalarType standardtypes::GetWStringType()
{
	ScalarType type;
	type.Name = "WString";
	type.Compare = WStringCompare;
	type.Free = NULL;
	type.Size = sizeof(fs::wstring);
	type.ToString = WStringString;
	type.Hash = WStringHash;
	type.IndexOf = CompareIndexOf<fs::wstring, WStringCompare>;
	//type.IndexOf = ScaledIndexOf<fs::wstring>;
	type.CopyIn = CopyToArray<fs::wstring>;
	type.Encode = Encode<fs::wstring>;
	type.Decode = Decode<fs::wstring>;
	return type;
}

// String type
int standardtypes::StringCompare(const void* left, const void* right)
{
	return ((fs::string*)left)->compare(*(fs::string*)right);
}

fs::wstring standardtypes::StringString(const void* item)
{
	//wstring ws(((fs::string*)item)->begin(),((fs::string*)item)->end());
	return wstring(L"Broken");
}

size_t  standardtypes::StringHash(const void* item)
{
	static std::hash<fs::string> hash;
	return hash(*(fs::string*)item);
}

ScalarType standardtypes::GetStringType()
{
	ScalarType type;
	type.Name = "String";
	type.Compare = StringCompare;
	type.Free = NULL;
	type.Size = sizeof(fs::string);
	type.ToString = StringString;
	type.Hash = StringHash;
	type.IndexOf = CompareIndexOf<fs::string, StringCompare>;
	//type.IndexOf = ScaledIndexOf<fs::wstring>;
	type.CopyIn = CopyToArray<fs::string>;
	type.Encode = Encode<fs::string>;
	type.Decode = Decode<fs::string>;
	return type;
}

// Long ScalarType
int standardtypes::LongCompare(const void* left, const void* right)
{
	return (*(long long*)left - *(long long*)right);
}

fs::wstring standardtypes::LongString(const void* item)
{
	long long temp = *(long long *)item;
	fs::wstring converted = to_wstring(temp);
	return converted;
}

size_t standardtypes::LongHash(const void* item)
{
	static std::hash<long long> hash;
	return hash(*(long long*)item);
}

ScalarType standardtypes::GetLongType()
{
	ScalarType type;
	type.Name = "Long";
	type.Compare = LongCompare;
	type.Free = NULL;
	type.Size = sizeof(long long);
	type.ToString = LongString;
	type.Hash = LongHash;
	type.IndexOf = TargetedIndexOf<long long>;
	type.CopyIn = CopyToArray<long long>;
	type.Encode = Encode<long long>;
	type.Decode = Decode<long long>;
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
	type.Name = "Int";
	type.Compare = IntCompare;
	type.Free = NULL;
	type.Size = sizeof(int);
	type.ToString = IntString;
	type.IndexOf = CompareIndexOf<int, IntCompare>;
	type.CopyIn = CopyToArray<int>;
	type.Hash = IntHash;
	type.Encode = Encode<int>;
	type.Decode = Decode<int>;
	return type;
}

size_t standardtypes::IntHash(const void* item)
{
	static std::hash<int> hash;
	return hash(*(int*)item);
}

// Bool ScalarType
int standardtypes::BoolCompare(const void* left, const void* right)
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

fs::wstring standardtypes::BoolString(const void* item)
{
	wstringstream result;
	result << *(bool*)item;
	return result.str();
}

ScalarType standardtypes::GetBoolType()
{
	ScalarType type;
	type.Name = "Bool";
	type.Compare = BoolCompare;
	type.Free = NULL;
	type.Size = sizeof(bool);
	type.ToString = BoolString;
	type.IndexOf = CompareIndexOf<bool, BoolCompare>;
	type.CopyIn = CopyToArray<bool>;
	type.Hash = BoolHash;
	type.Encode = Encode<bool>;
	type.Decode = Decode<bool>;
	return type;
}

size_t standardtypes::BoolHash(const void* item)
{
	static std::hash<bool> hash;
	return hash(*(bool*)item);
}

//HashSet Type
ScalarType standardtypes::GetHashSetType()
{
	ScalarType type;
	type.Name = "HashSet";
	type.Compare = NULL;
	type.Free = IndirectDelete;
	type.Size = sizeof(std::hash_set<void*, ScalarType, ScalarType>*);
	type.ToString = NULL;
	type.CopyIn = CopyToArray<std::hash_set<void*, ScalarType, ScalarType>*>;
	return type;
}

ScalarType standardtypes::GetKeyVectorType()
{
	ScalarType type;
	type.Name = "KeyVector";
	type.Compare = NULL;
	type.Free = IndirectDelete;
	type.Size = sizeof(fs::KeyVector*);
	type.ToString = NULL;
	type.CopyIn = CopyToArray<fs::KeyVector*>;
	return type;
}


