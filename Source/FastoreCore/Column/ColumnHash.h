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

typedef eastl::hash_set<void*, ScalarType, ScalarType> ColumnHashSet;
typedef eastl::hash_set<void*, ScalarType, ScalarType>::const_iterator ColumnHashSetConstIterator;
typedef eastl::hash_map<void*, Leaf*, ScalarType, ScalarType> ColumnHashMap;
typedef eastl::hash_map<void*, Leaf*, ScalarType, ScalarType>::iterator ColumnHashMapIterator;
typedef eastl::pair <void*, Leaf*> RowLeafPair;

class ColumnHash : public ColumnBuffer
{
	public:
		ColumnHash(const ScalarType rowType, const ScalarType valueType);
		void* GetValue(void* rowId);
		void* Include(void* value, void* rowId);
		void* Exclude(void* value, void* rowId);
		// TODO: what should updatevalue return?  All the row IDs?
		void UpdateValue(void* oldValue, void* newValue);
		GetResult GetRows(Range);

	private:
		void ValuesMoved(void* ,Leaf*);
		eastl::vector<eastl::pair<void*,void*>> BuildData(BTree::iterator, BTree::iterator, void*, bool, int, bool&);
		ScalarType _rowType;
		ScalarType _valueType;
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
	_valueType = valueType;
	_rows = new ColumnHashMap(32, _rowType, _rowType);
	_values = new BTree(_valueType, GetHashSetType());
	_values->setValuesMovedCallback(
		[this](void* value, Leaf* newLeaf) -> void
		{
			this->ValuesMoved(value, newLeaf);
		});
}

inline void* ColumnHash::GetValue(void* rowId)
{
	ColumnHashMapIterator iterator;
	iterator = _rows->find(rowId);
	
	if (iterator != _rows->end())
	{
		Leaf* leaf = iterator->second;
		return leaf->GetKey(
			[rowId](void* hash) -> bool
			{
				ColumnHashSet* newrows = (ColumnHashSet*)(*(void**)hash);
				return newrows->find(rowId) != newrows->end();
			});
	}
	else
	{
		return NULL;
	}
}

inline void* ColumnHash::Include(void* value, void* rowId)
{
	Leaf* valueLeaf;
	ColumnHashSet* newrows = new ColumnHashSet(32, _rowType, _rowType); //Path provides an enhancement here as well.. Insert can return the path to the correct element, and then we can decide whether to construct the hash set.
	void* existing = _values->Insert(value, &newrows, &valueLeaf);
	if (existing != NULL)
	{
		delete newrows;
		newrows = (ColumnHashSet*)(*(void**)existing);
	}

	//TODO: Correct Return types (undo information)
	//return true if it's a new element.
	if (newrows->insert(rowId).second)
	{
		_rows->insert(RowLeafPair(rowId, valueLeaf));
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
	Leaf* valueLeaf;
	void* existing = _values->Insert(newValue, valuesToMove, &valueLeaf);

	if (existing != NULL)
	{
		//merge old and new
		ColumnHashSet* existingValues = (ColumnHashSet*)(*(void**)existing);

		ColumnHashSetConstIterator iterator;
		iterator = valuesToMove->begin();

		while (iterator != valuesToMove->end())
		{
			existingValues->insert(*iterator);
			ValuesMoved(*iterator, valueLeaf);
		}
	}
	else
	{
		ColumnHashSetConstIterator iterator;
		iterator = valuesToMove->begin();

		while (iterator != valuesToMove->end())
		{
			ValuesMoved(*iterator, valueLeaf);
		}
	}
}

inline void ColumnHash::ValuesMoved(void* value, Leaf* leaf)
{
	ColumnHashSet* existingValues = (ColumnHashSet*)(*(void**)value);

	auto start = existingValues->begin();
	auto end = existingValues->end();

	while(start != end)
	{
		_rows->find(*start)->second = leaf;
		start++;
	}	
}

inline GetResult ColumnHash::GetRows(Range range)
{
	//These may not exist, add logic for handling that.
	GetResult result;
	RangeBound start = *(range.Start);
	RangeBound end = *(range.End);

	//Figure out forward. 
	bool firstMatch = true; //Seeking to beginning or end
	bool lastMatch = true;
	BTree::iterator first = range.Start.HasValue() ? _values->find(start.Value, firstMatch) : _values->begin();
	BTree::iterator last =  range.End.HasValue() ? _values->find(end.Value, lastMatch) : _values->end();

	//If there was a match, we're exclusive we need to move one forward. If there was not a match, we are already pointing at the next highest value.
	if ((!start.Inclusive && firstMatch))
		first++;

	if ((!end.Inclusive && lastMatch) || !lastMatch)
		last--;

	void* startID = start.RowId.HasValue() ? *start.RowId : NULL;
	void* endID = end.RowId.HasValue() ? *end.RowId : NULL;

	result.Data = BuildData(first, last, startID != NULL ? startID : endID, range.Ascending, range.Limit > range.MaxLimit? range.MaxLimit : range.Limit, result.Limited);

	return result;
}

inline eastl::vector<eastl::pair<void*,void*>> ColumnHash::BuildData(BTree::iterator first, BTree::iterator last, void* startId, bool ascending, int limit, bool &limited)
{
	//Assumption -- Repeated calls will come in in the same direction, therefore we don't need to check both start and end
	
	int num = 0;
	bool foundCurrentId = startId == NULL;
    limited = false;
	eastl::vector<eastl::pair<void*,void*>> rows;

	if (!ascending)
	{
		BTree::iterator temp = first;
		first = last;
		last = temp;
	}

	while (first != last && !limited)
	{
		auto rowIds = (ColumnHashSet*)*(void**)((*first).second);
		
		auto idStart = rowIds->begin();
		auto idEnd = rowIds->end();

		//Assumption.. The Id will exist in the first value we pull
		//Otherwise either our tree has changed (we are on the wrong revision) or we have an invalid start id, and this loop will never terminate. Ever. Until the end of time.
		while (!foundCurrentId)
		{
			if (*idStart == startId)
				foundCurrentId = true;

			idStart++;				
		}

		while (idStart != idEnd)
		{
			rows.push_back(eastl::pair<void*,void*>(*idStart,(*first).first));
			idStart++;
			num++;
			if(num == limit)
			{
				limited = true; break;
			}
		}

		if (ascending)
			first++;
		else
			first--;
	}

	return rows;
}
