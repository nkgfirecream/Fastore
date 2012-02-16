#include "Schema\standardtypes.h"
#include <sstream>
#include "EASTL\functional.h"
#include "EASTL\hash_set.h"

using namespace std;

void standardtypes::IndirectDelete(void* item)
{
	delete *(void**)item;
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
	return type;
}

// Long ScalarType
int standardtypes::LongCompare(const void* left, const void* right)
{
	return *(long*)left - *(long*)right;
}

fs::wstring standardtypes::LongString(const void* item)
{
	wstringstream result;
	result << *(long*)item;
	return result.str();
}

size_t standardtypes::LongHash(const void* item)
{
	static eastl::hash<long> hash;
	return hash(*(long*)item);
}

ScalarType standardtypes::GetLongType()
{
	ScalarType type;
	type.Compare = LongCompare;
	type.Free = NULL;
	type.Size = sizeof(long);
	type.ToString = LongString;
	type.Hash = LongHash;
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


