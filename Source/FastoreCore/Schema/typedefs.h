#pragma once
#include <EASTL\string.h>
#include <EASTL\hash_map.h>
#include <EASTL\hash_set.h>

class Leaf;

//Fastore string?
typedef std::wstring fstring;

//Columnhash
typedef eastl::hash_set<void*> ColumnHashSet;
typedef eastl::hash_set<void*>::const_iterator ColumnHashSetConstIterator;

typedef eastl::hash_map<void*, Leaf&> ColumnHashMap;
typedef eastl::hash_map<void*, Leaf&>::iterator ColumnHashMapIterator;

typedef eastl::pair <void*, Leaf&> ValueLeafPair;
