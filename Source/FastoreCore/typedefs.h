#pragma once
#include <string>
#include <vector>

namespace fs
{
	typedef std::wstring wstring;
	typedef std::string string;
	typedef void* Value;
	typedef void* Key;

	typedef std::vector<Value> ValueVector;
	typedef std::vector<Key> KeyVector;

	typedef std::pair<Value, Key> ValueKey;
	typedef std::pair<Value, KeyVector> ValueKeys;
	typedef std::vector<ValueKey> ValueKeyVector;
	typedef std::vector<ValueKeys> ValueKeysVector;
	typedef std::vector<ValueKeysVector> ValueKeysVectorVector;
	typedef std::vector<KeyVector> KeyVectorVector;	
}