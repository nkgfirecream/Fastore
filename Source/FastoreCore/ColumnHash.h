#pragma once
#include "Schema\type.h"
#include "Schema\typedefs.h"
#include "BTree.h"
#include "BTreeObserver.h"


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
		ColumnHashMap* _rows;
		BTree* _values;
};


inline ColumnHash::ColumnHash(const type rowType, const type valueType)
{
	//TODO: Hash of rowType
	_values = new BTree(valueType, rowType);

	_rows = new ColumnHashMap();
}


inline void* ColumnHash::GetValue(void* value)
{
	ColumnHashMapIterator iterator;
	iterator = _rows->find(value);
	
	if (iterator != _rows->end())
	{
		Leaf** leaf = iterator->second;
		return (*leaf)->GetKey(
			[value](void* hash) -> bool
			{
				ColumnHashSet* newrows = (ColumnHashSet*)hash;
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
	Leaf** valueLeaf;
	ColumnHashSet* newrows = new ColumnHashSet();
	void* existing = _values->Insert(value, &newrows, valueLeaf);
	if(existing != NULL)
		newrows = (ColumnHashSet*)existing;

	//TODO: Correct Return types (undo information)
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
	//TODO: BTree needs delete for this.
	return NULL;
}

inline void ColumnHash::UpdateValue(void* oldValue, void* newValue)
{
	//TODO: Add remove to BTree
	ColumnHashSet* valuesToMove; // == (eastl::hash_set<void*>*)_values->Remove(oldValue);
	Leaf** valueLeaf;
	void* existing = _values->Insert(newValue, valuesToMove, valueLeaf);

	if (existing != NULL)
	{
		//merge old and new
		ColumnHashSet* existingValues = (eastl::hash_set<void*>*)existing;

		ColumnHashSetConstIterator iterator;
		iterator = valuesToMove->begin();

		while (iterator !=  valuesToMove->end())
		{
			existingValues->insert(*iterator);
			ValuesMoved(*iterator, valueLeaf);
		}
	}
	else
	{
		ColumnHashSetConstIterator iterator;
		iterator = valuesToMove->begin();

		while (iterator !=  valuesToMove->end())
		{
			ValuesMoved(*iterator, valueLeaf);
		}
	}
}

inline void ColumnHash::ValuesMoved(void* value, Leaf** leaf)
{
	_rows->find(value)->second = leaf;
}