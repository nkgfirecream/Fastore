#pragma once
#include "Schema\type.h"
#include "EASTL\hash_set.h"
#include "EASTL\hash_map.h"
#include "BTree.h"
#include "BTreeObserver.h"
#include <hash_map>

using namespace eastl;


class ColumnHash
{
	public:
		ColumnHash(const type rowType, const type valueType);
		void* GetValue(void* value);
		void* Include(void* value, void* rowID);
		void* Exclude(void* value, void* rowID);
		// TODO: what should updatevalue return?  All the row IDs?
		void UpdateValue(void* oldValue, void* newValue);

	private:
		void ValuesMoved(void* value, Leaf** newLeaf);
		eastl::hash_map<void*, Leaf**>* _rows;
		BTree* _values;
};



inline ColumnHash::ColumnHash(const type rowType, const type valueType)
{
	//TODO: Hash of rowType
	_values = new BTree(valueType, rowType);

	_rows = new eastl::hash_map<void*, Leaf**>();
}


inline void* ColumnHash::GetValue(void* value)
{
	eastl::hash_map <void*, Leaf**> :: const_iterator hash_mapIterator;
	hash_mapIterator = _rows->find(value);
	
	if (hash_mapIterator != _rows->end())
	{
		Leaf** leaf = hash_mapIterator->second;
		return (*leaf)->GetKey(
			[value](void* hash) -> bool
			{
				eastl::hash_set<void*>* newrows = (eastl::hash_set<void*>*)hash;
				return newrows->find(value) != newrows->end();
			});
	}
	else
	{
		return NULL;
	}
}

inline void* ColumnHash::Include(void* value, void* rowId)
{
	typedef eastl::pair <void*,Leaf**> ValueLeafPair;
	Leaf** valueLeaf;
	eastl::hash_set<void*>* newrows = new eastl::hash_set<void*>();
	void* existing = _values->Insert(value, &newrows, valueLeaf);
	if(existing != NULL)
		newrows = (eastl::hash_set<void*>*)existing;

	//TODO: Correct Return types
	if (newrows->insert(rowId).second)
	{
		_rows->insert(ValueLeafPair (rowId, valueLeaf));
		return NULL;
	}
	else
		return NULL;

}

inline void* ColumnHash::Exclude(void* value, void* rowId)
{
	return NULL;
}

inline void ColumnHash::UpdateValue(void* oldValue, void* newValue)
{
	//TODO: Add remove to BTree
	eastl::hash_set<void*>* valuesToMove; // == (eastl::hash_set<void*>*)_values->Remove(oldValue);
	Leaf** valueLeaf;
	void* existing = _values->Insert(newValue, valuesToMove, valueLeaf);

	if (existing != NULL)
	{
		//merge old and new
		eastl::hash_set<void*>* existingValues = (eastl::hash_set<void*>*)existing;

		eastl::hash_set<void*> :: iterator hash_SetIterator;
		hash_SetIterator = valuesToMove->begin();

		while (hash_SetIterator !=  valuesToMove->end())
		{
			existingValues->insert(*hash_SetIterator);
			ValuesMoved(*hash_SetIterator, valueLeaf);
		}
	}
	else
	{
		eastl::hash_set<void*> :: iterator hash_SetIterator;
		hash_SetIterator = valuesToMove->begin();

		while (hash_SetIterator !=  valuesToMove->end())
		{
			ValuesMoved(*hash_SetIterator, valueLeaf);
		}
	}
}

inline void ColumnHash::ValuesMoved(void* value, Leaf** leaf)
{
	_rows->find(value)->second = leaf;
}