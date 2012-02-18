#pragma once

#include <EASTL\hash_set.h>
#include <EASTL\hash_map.h>
#include "..\Schema\standardtypes.h"
#include "..\BTree.h"
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

inline ColumnHash::ColumnHash(const ScalarType rowType, const ScalarType valueType)
{
	_rowType = rowType;
	_valueType = valueType;
	_rows = new ColumnHashMap(32, _rowType, _rowType);
	_values = new BTree(_valueType, standardtypes::GetHashSetType());
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
				return (*(ColumnHashSet**)hash)->find(rowId) != (*(ColumnHashSet**)hash)->end();
			});
	}
	else
	{
		return NULL;
	}
}

inline void* ColumnHash::Include(void* value, void* rowId)
{
	//TODO: Use of Path to avoid two trips down tree
	//TODO: Return Undo Information
	Leaf* valueLeaf;
	ColumnHashSet* existing = *(ColumnHashSet**)_values->GetValue(value, &valueLeaf);
	if (existing != NULL)
	{
		if(existing->insert(rowId).second)
		{
			_rows->insert(RowLeafPair(rowId, valueLeaf));
		}

		return NULL;
	}
	else
	{
		ColumnHashSet* newRows = new ColumnHashSet(32, _rowType, _rowType);
		newRows->insert(rowId);
		_values->Insert(value, newRows, &valueLeaf);
		_rows->insert(RowLeafPair(rowId, valueLeaf));

		return NULL;
	}
}

inline void* ColumnHash::Exclude(void* value, void* rowId)
{
	Leaf* valueLeaf;
	ColumnHashSet* existing = *(ColumnHashSet**)_values->GetValue(value, &valueLeaf);
	//If existing is NULL, that row id did not exist under that value
	if (existing != NULL)
	{
		existing->erase(rowId);
		if(existing->size() == 0)
		{
			_values->Delete(value);
			delete(existing);
		}
		_rows->erase(rowId);
	}
	
	return NULL;
}

inline void ColumnHash::UpdateValue(void* oldValue, void* newValue)
{
	Leaf* valueLeaf;
	ColumnHashSet* valuesToMove = *(ColumnHashSet**)_values->GetValue(oldValue, &valueLeaf);

	_values->Delete(oldValue);

	ColumnHashSet* existing = *(ColumnHashSet**)_values->Insert(newValue, valuesToMove, &valueLeaf);

	if (existing != NULL)
	{
		//merge old and new
		ColumnHashSetConstIterator iterator;
		iterator = valuesToMove->begin();

		while (iterator != valuesToMove->end())
		{
			existing->insert(*iterator);			
		}

		ValuesMoved(existing, valueLeaf);

		delete(valuesToMove);
	}
	else
	{
		ValuesMoved(valuesToMove, valueLeaf);
	}
}

inline void ColumnHash::ValuesMoved(void* value, Leaf* leaf)
{
	ColumnHashSet* existingValues = *(ColumnHashSet**)(value);

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

	if(range.Start.HasValue() && range.End.HasValue())
	{
		//Bounds checking
		if(last == first && (!end.Inclusive || !start.Inclusive))
		{
			//We have only one result, and it is excluded
			return result; //Empty result.
		}

		if(_valueType.Compare(start.Value, end.Value) > 0)
		{
			//Start is after end. Invalid input.
			throw;
		}
	}

	//Swap iterators if descending
	if (range.Ascending)
	{
		//Adjust iteratos
		//Last needs to point to the element AFTER the last one we want to get
		if (lastMatch && end.Inclusive)
		{
			last++;
		}

		//First needs to point to the first element we DO want to pick up
		if (firstMatch && !start.Inclusive)
		{
			first++;
		}	
	}		
	else
	{
		//If we are descending, lasts needs to point at the first element we want to pick up
		if (!lastMatch || (lastMatch && !end.Inclusive))
		{
			//If we are pointing at an excluded element, move back
			last--;
		} 

		//If we are descending, first need to point at the element BEFORE the last one we want to pick up
		if(!firstMatch || (firstMatch && start.Inclusive))
		{
			first--;
		}

		//Swap iterators
		//TODO: Non-Copy iterator swapping
		BTree::iterator temp = first;
		first = last;
		last = temp;
	}

	//Assumption -- Repeated calls will come in in the same direction, therefore we don't need to check both start and end
	result.Data = BuildData(first, last, start.RowId.HasValue() ? *start.RowId :  end.RowId.HasValue() ? *end.RowId : NULL, range.Ascending, range.Limit > range.MaxLimit? range.MaxLimit : range.Limit, result.Limited);

	return result;
}

inline eastl::vector<eastl::pair<void*,void*>> ColumnHash::BuildData(BTree::iterator first, BTree::iterator last, void* startId, bool ascending, int limit, bool &limited)
{	
	int num = 0;
	bool foundCurrentId = startId == NULL;
    limited = false;
	eastl::vector<eastl::pair<void*,void*>> rows;	
	
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
