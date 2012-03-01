#pragma once

#include "..\typedefs.h"
#include <EASTL\hash_set.h>
#include <EASTL\hash_map.h>
#include "..\Schema\standardtypes.h"
#include "..\BTree.h"
#include "..\Column\columnbuffer.h"
#include <EASTL\sort.h>

using namespace eastl;

typedef eastl::hash_set<Key, ScalarType, ScalarType> ColumnHashSet;
typedef eastl::hash_set<Key, ScalarType, ScalarType>::const_iterator ColumnHashSetConstIterator;
typedef eastl::hash_map<Key, Leaf*, ScalarType, ScalarType> ColumnHashMap;
typedef eastl::hash_map<Key, Leaf*, ScalarType, ScalarType>::iterator ColumnHashMapIterator;
typedef eastl::pair <Key, Leaf*> RowLeafPair;
typedef eastl::hash_map<Value, KeyVector, ScalarType, ScalarType> ValueKeysHashMap;

class ColumnHash : public ColumnBuffer
{
	public:
		ColumnHash(const ScalarType rowType, const ScalarType valueType);
		ValueVector GetValues(KeyVector rowId);
		Value Include(Value value, Key rowId);
		Value Exclude(Value value, Key rowId);
		GetResult GetRows(Range);
		ValueKeysVectorVector GetSorted(KeyVectorVector input);

	private:
		void ValuesMoved(void*, Leaf*);
		Value GetValue(Key rowId);
		ValueKeysVector BuildData(BTree::iterator, BTree::iterator, void*, bool, int, bool&);
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

inline ValueVector ColumnHash::GetValues(KeyVector rowIds)
{
	ValueVector values(rowIds.size());
	for (int i = 0; i < rowIds.size(); i++)
	{
		values[i] = GetValue(rowIds[i]);
	}

	return values;
}

inline Value ColumnHash::GetValue(Key rowId)
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

inline ValueKeysVectorVector ColumnHash::GetSorted(KeyVectorVector input)
{
	ValueKeysVectorVector result;
	for(int i = 0; i < input.size(); i++)
	{
		//Create temporary hash for values and keys...
		ValueKeysHashMap hash(32, _valueType, _valueType);

		BTree valueKeyTree(_valueType, standardtypes::GetKeyVectorType());
		
		KeyVector keys;
		//insert each key into the hash for its correct value;
		for(int j = 0; j < keys.size(); j++)
		{
			Key key = keys[i];
			Value val = GetValue(key);
						
			BTree::Path path = valueKeyTree.GetPath(val);
			if(path.Match)
			{
				KeyVector* existing = *(KeyVector**)(*path.Leaf)[path.LeafIndex].second;
				existing->push_back(key);
			}
			else
			{
				KeyVector* vector = new KeyVector();
				vector->push_back(key);
				valueKeyTree.Insert(path, val, vector);
			}
		}

		ValueKeysVector sorted;

		auto start = valueKeyTree.begin();
		auto end = valueKeyTree.end();

		while(start != end)
		{
			sorted.push_back(ValueKeys((*start).first, **((KeyVector**)(*start).second)));
			++start;
		}

		//Stick that list into the result
		result.push_back(sorted);
	}

	return result;
}

inline Value ColumnHash::Include(Value value, Key rowId)
{
	//TODO: Return Undo Information
	BTree::Path  path = _values->GetPath(value);
	if (path.Match)
	{
		ColumnHashSet* existing = *(ColumnHashSet**)(*path.Leaf)[path.LeafIndex].second;
		if (existing->insert(rowId).second)
		{
			_rows->insert(RowLeafPair(rowId, path.Leaf));
		}

		return NULL;
	}
	else
	{
		ColumnHashSet* newRows = new ColumnHashSet(32, _rowType, _rowType);
		newRows->insert(rowId);
		_rows->insert(RowLeafPair(rowId, path.Leaf));
		//Insert may generate a different leaf that the value gets inserted into,
		//so the above may be incorrect momentarily. If the value gets inserted
		//on a new split, the callback will be run and change the entry.
		_values->Insert(path, value, &newRows);

		return NULL;
	}
}

inline Value ColumnHash::Exclude(Value value, Key rowId)
{
	BTree::Path  path = _values->GetPath(value);
	//If existing is NULL, that row id did not exist under that value
	if (path.Match)
	{
		ColumnHashSet* existing = *(ColumnHashSet**)(*path.Leaf)[path.LeafIndex].second;
		existing->erase(rowId);
		if(existing->size() == 0)
		{
			_values->Delete(path);
			delete(existing);
		}
		_rows->erase(rowId);
	}
	
	return NULL;
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

	bool firstMatch = true; //Seeking to beginning or end
	bool lastMatch = true;
	BTree::iterator first = range.Start.HasValue() ? _values->find(start.Value, firstMatch) : _values->begin();
	BTree::iterator last =  range.End.HasValue() ? _values->find(end.Value, lastMatch) : _values->end();

	if(range.Start.HasValue() && range.End.HasValue())
	{
		//Bounds checking
		//TODO: Is this needed? Could the BuildData logic handle this correctly?
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

inline ValueKeysVector ColumnHash::BuildData(BTree::iterator first, BTree::iterator last, Key startId, bool ascending, int limit, bool &limited)
{	
	int num = 0;
	bool foundCurrentId = startId == NULL;
    limited = false;
	ValueKeysVector rows;	
	
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

		KeyVector keys;
		while (idStart != idEnd)
		{
			keys.push_back(*idStart);
			idStart++;
			num++;
			if(num == limit)
			{
				limited = true; break;
			}
		}

		if(keys.size() > 0)
		{
			rows.push_back(ValueKeys((*first).first, keys));
		}		

		if (ascending)
			first++;
		else
			first--;
	}

	return rows;
}
