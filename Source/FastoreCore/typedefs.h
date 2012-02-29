#pragma once
#include <EASTL\string.h>
#include <EASTL\vector.h>

namespace fs
{
	typedef std::wstring wstring;
	typedef std::string string;
	typedef void* Value;
	typedef void* Key;

	typedef eastl::vector<Value> ValueVector;
	typedef eastl::vector<Key> KeyVector;

	typedef eastl::pair<Value, Key> ValueKey;
	typedef eastl::pair<Value, KeyVector> ValueKeys;
	typedef eastl::vector<ValueKey> ValueKeyVector;
	typedef eastl::vector<ValueKeys> ValueKeysVector;
	typedef eastl::vector<ValueKeysVector> ValueKeysVectorVector;
	typedef eastl::vector<KeyVector> KeyVectorVector;
}