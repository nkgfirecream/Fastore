#pragma once

#include "..\typedefs.h"
#include <EASTL\hash_set.h>
#include <EASTL\hash_map.h>
#include "..\Schema\standardtypes.h"
#include "..\BTree.h"
#include "..\Column\IColumnBuffer.h"
#include <EASTL\sort.h>

using namespace eastl;

const int HashBufferRowMapInitialSize = 32;

class HashBuffer : public IColumnBuffer
{
	public:
		HashBuffer(const ScalarType& rowType, const ScalarType &valueType, const fs::wstring& name);
		ValueVector GetValues(const KeyVector& rowId);
		bool Include(Value value, Key rowId);
		bool Exclude(Value value, Key rowId);
		GetResult GetRows(Range& range);
		ValueKeysVectorVector GetSorted(const KeyVectorVector& input);

		ScalarType GetRowType();
		ScalarType GetKeyType();
		fs::wstring GetName();
		bool GetUnique();
		bool GetRequired();

	private:
		typedef eastl::hash_set<Key, ScalarType, ScalarType> HashBufferHashSet;
		typedef eastl::hash_set<Key, ScalarType, ScalarType>::const_iterator HashBufferHashSetIterator;
		typedef eastl::hash_map<Key, Node*, ScalarType, ScalarType> HashBufferHashMap;
		typedef eastl::hash_map<Key, Node*, ScalarType, ScalarType>::iterator HashBufferHashMapIterator;
		typedef eastl::pair <Key, Node*> RowLeafPair;

		void ValuesMoved(void*, Node*);
		Value GetValue(Key rowId);
		ValueKeysVector BuildData(BTree::iterator&, BTree::iterator&, void*, bool, int, bool&);
		ScalarType _rowType;
		ScalarType _valueType;
		HashBufferHashMap* _rows;
		BTree* _values;
		fs::wstring _name;
		bool _required;
};

inline HashBuffer::HashBuffer(const ScalarType& rowType, const ScalarType& valueType,const fs::wstring& name)
{
	_name = name;
	_rowType = rowType;
	_valueType = valueType;
	_rows = new HashBufferHashMap(HashBufferRowMapInitialSize, _rowType, _rowType);
	_values = new BTree(_valueType, standardtypes::GetHashSetType());
	_required = false;
	_values->setValuesMovedCallback
	(
		[this](void* value, Node* newLeaf) -> void
		{
			this->ValuesMoved(value, newLeaf);
		}
	);
}

inline fs::wstring HashBuffer::GetName()
{
	return _name;
}

inline ScalarType HashBuffer::GetKeyType()
{
	return _valueType;
}

inline ScalarType HashBuffer::GetRowType()
{
	return _rowType;
}

inline bool HashBuffer::GetUnique()
{
	return false;
}

inline bool HashBuffer::GetRequired()
{
	return _required;
}

inline ValueVector HashBuffer::GetValues(const KeyVector& rowIds)
{
	ValueVector values(rowIds.size());
	for (unsigned int i = 0; i < rowIds.size(); i++)
	{
		values[i] = GetValue(rowIds[i]);
	}

	return values;
}

inline Value HashBuffer::GetValue(Key rowId)
{
	HashBufferHashMapIterator iterator;
	iterator = _rows->find(rowId);
	
	if (iterator != _rows->end())
	{
		Node* leaf = iterator->second;
		return leaf->GetKey
		(
			[rowId](void* hash) -> bool
			{
				return (*(HashBufferHashSet**)hash)->find(rowId) != (*(HashBufferHashSet**)hash)->end();
			}
		);
	}
	else
	{
		return NULL;
	}
}

inline ValueKeysVectorVector HashBuffer::GetSorted(const KeyVectorVector& input)
{
	ValueKeysVectorVector result;
	for (unsigned int i = 0; i < input.size(); i++)
	{
		BTree valueKeyTree(_valueType, standardtypes::GetKeyVectorType());
		
		KeyVector keys;
		//insert each key into the hash for its correct value;
		for (unsigned int j = 0; j < keys.size(); j++)
		{
			Key key = keys[i];
			Value val = GetValue(key);
						
			BTree::Path path = valueKeyTree.GetPath(val);
			if (path.Match)
			{
				KeyVector* existing = *(KeyVector**)(*path.Leaf)[path.LeafIndex].value;
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

		while (start != end)
		{
			sorted.push_back(ValueKeys((*start).key, **((KeyVector**)(*start).value)));
			++start;
		}

		//Stick that list into the result
		result.push_back(sorted);
	}

	return result;
}

inline bool HashBuffer::Include(Value value, Key rowId)
{
	//TODO: Return Undo Information
	BTree::Path  path = _values->GetPath(value);
	if (path.Match)
	{
		HashBufferHashSet* existing = *(HashBufferHashSet**)(*path.Leaf)[path.LeafIndex].value;
		if (existing->insert(rowId).second)
		{
			_rows->insert(RowLeafPair(rowId, path.Leaf));
			return true;
		}
		return false;
	}
	else
	{
		HashBufferHashSet* newRows = new HashBufferHashSet(32, _rowType, _rowType);
		newRows->insert(rowId);
		_rows->insert(RowLeafPair(rowId, path.Leaf));
		//Insert may generate a different leaf that the value gets inserted into,
		//so the above may be incorrect momentarily. If the value gets inserted
		//on a new split, the callback will be run and change the entry.
		_values->Insert(path, value, &newRows);

		return true;
	}
}

inline bool HashBuffer::Exclude(Value value, Key rowId)
{
	BTree::Path  path = _values->GetPath(value);
	//If existing is NULL, that row id did not exist under that value
	if (path.Match)
	{
		HashBufferHashSet* existing = *(HashBufferHashSet**)(*path.Leaf)[path.LeafIndex].value;
		bool result = existing->erase(rowId) == -1;
		if (result)
		{
			if (existing->size() == 0)
			{
				_values->Delete(path);
				delete(existing);
			}
			_rows->erase(rowId);
		}
		return result;
	}
	
	return false;
}

inline void HashBuffer::ValuesMoved(void* value, Node* leaf)
{
	HashBufferHashSet* existingValues = *(HashBufferHashSet**)(value);

	auto start = existingValues->begin();
	auto end = existingValues->end();

	while (start != end)
	{
		_rows->find(*start)->second = leaf;
		start++;
	}	
}

inline GetResult HashBuffer::GetRows(Range& range)
{
	//These may not exist, add logic for handling that.
	GetResult result;

	bool firstMatch = true; //Seeking to beginning or end
	bool lastMatch = true;
	BTree::iterator first = range.Start.HasValue() ? _values->find((*range.Start).Value, firstMatch) : _values->begin();
	BTree::iterator last =  range.End.HasValue() ? _values->find((*range.End).Value, lastMatch) : _values->end();

	if (range.Start.HasValue() && range.End.HasValue())
	{
		RangeBound start = *range.Start;
		RangeBound end = *range.End;

		//Bounds checking
		//TODO: Is this needed? Could the BuildData logic handle this correctly?
		if (last == first && (!end.Inclusive || !start.Inclusive))
		{
			//We have only one result, and it is excluded
			return result; //Empty result.
		}

		if (_valueType.Compare(start.Value, end.Value) > 0)
		{
			//Start is after end-> Invalid input.
			throw;
		}
	}

	//Swap iterators if descending
	if (range.Ascending)
	{
		//Adjust iterators
		//Last needs to point to the element AFTER the last one we want to get
		if (lastMatch && range.End.HasValue() && (*range.End).Inclusive)
		{
			last++;
		}

		//First needs to point to the first element we DO want to pick up
		if (firstMatch && !(range.Start.HasValue() && (*range.Start).Inclusive))
		{
			first++;
		}	
	}		
	else
	{
		//If we are descending, lasts needs to point at the first element we want to pick up
		if (!lastMatch || (lastMatch && range.End.HasValue() && !(*range.End).Inclusive))
		{
			//If we are pointing at an excluded element, move back
			last--;
		} 

		//If we are descending, first need to point at the element BEFORE the last one we want to pick up
		if (!firstMatch || (firstMatch && range.Start.HasValue() && (*range.Start).Inclusive))
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
	result.Data = 
		BuildData
		(
			first, 
			last, 
			range.Start.HasValue() && (*range.Start).RowId.HasValue() 
				? *(*range.Start).RowId 
				: range.End.HasValue() && (*range.End).RowId.HasValue() 
					? *(*range.End).RowId
					: NULL, 
			range.Ascending, 
			range.Limit > range.MaxLimit
				? range.MaxLimit 
				: range.Limit, 
			result.Limited
		);

	return result;
}

inline ValueKeysVector HashBuffer::BuildData(BTree::iterator& first, BTree::iterator& last, Key startId, bool ascending, int limit, bool &limited)
{	
	int num = 0;
	bool foundCurrentId = startId == NULL;
    limited = false;
	ValueKeysVector rows;	
	
	while (first != last && !limited)
	{
		auto rowIds = (HashBufferHashSet*)*(void**)((*first).value);
		
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
			if (num == limit)
			{
				limited = true; break;
			}
		}

		if (keys.size() > 0)
		{
			rows.push_back(ValueKeys((*first).key, keys));
		}		

		if (ascending)
			first++;
		else
			first--;
	}

	return rows;
}
