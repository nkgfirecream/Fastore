#pragma once

#include <EASTL\hash_set.h>
#include <EASTL\hash_map.h>
#include "..\Schema\scalar.h"
#include "..\typedefs.h"
#include "..\Schema\standardtypes.h"
#include "..\BTree.h"
#include "..\Range.h"
#include "..\Column\columnbuffer.h"

using namespace eastl;

typedef eastl::hash_set<void*> ColumnHashSet;
typedef eastl::hash_set<void*>::const_iterator ColumnHashSetConstIterator;
typedef eastl::hash_map<void*, Leaf&> ColumnHashMap;
typedef eastl::hash_map<void*, Leaf&>::iterator ColumnHashMapIterator;
typedef eastl::pair <void*, Leaf&> ValueLeafPair;

class ColumnHash : public ColumnBuffer
{
	public:
		ColumnHash(const ScalarType rowType, const ScalarType valueType);
		void* GetValue(void* value);
		void* Include(void* value, void* rowID);
		void* Exclude(void* value, void* rowID);
		// TODO: what should updatevalue return?  All the row IDs?
		void UpdateValue(void* oldValue, void* newValue);
		GetResult GetRows(Range);

	private:
		void ValuesMoved(void*,Leaf&);
		ScalarType _rowType;
		ColumnHashMap* _rows;
		BTree* _values;
};

// HashSet type -- Can't be used as a keytype.
ScalarType GetHashSetType()
{
	ScalarType type;
	type.Compare = NULL;
	type.Free = IndirectDelete;
	type.Size = sizeof(ColumnHashSet*);
	type.ToString = NULL;
	return type;
}

inline ColumnHash::ColumnHash(const ScalarType rowType, const ScalarType valueType)
{
	_rowType = rowType;
	_rows = new ColumnHashMap();
	_values = new BTree(valueType, GetHashSetType());
	_values->setValuesMovedCallback(
		[this](void* value, Leaf& newLeaf) -> void
		{
			this->ValuesMoved(value, newLeaf);
		});
}

inline void* ColumnHash::GetValue(void* value)
{
	ColumnHashMapIterator iterator;
	iterator = _rows->find(value);
	
	if (iterator != _rows->end())
	{
		Leaf& leaf = iterator->second;
		return leaf.GetKey(
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
		_rows->insert(ValueLeafPair (rowId, **valueLeaf));
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
		ColumnHashSet* existingValues = (ColumnHashSet*)existing;

		ColumnHashSetConstIterator iterator;
		iterator = valuesToMove->begin();

		while (iterator != valuesToMove->end())
		{
			existingValues->insert(*iterator);
			ValuesMoved(*iterator, **valueLeaf);
		}
	}
	else
	{
		ColumnHashSetConstIterator iterator;
		iterator = valuesToMove->begin();

		while (iterator != valuesToMove->end())
		{
			ValuesMoved(*iterator, **valueLeaf);
		}
	}
}

inline void ColumnHash::ValuesMoved(void* value, Leaf& leaf)
{
	_rows->find(value)->second = leaf;
}

inline GetResult ColumnHash::GetRows(Range range)
{

}
