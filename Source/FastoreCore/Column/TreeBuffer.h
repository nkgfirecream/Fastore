#pragma once

#include "..\typedefs.h"
#include "..\Schema\standardtypes.h"
#include "..\BTree.h"
#include "..\KeyTree.h"
#include "..\Column\IColumnBuffer.h"

template<> void standardtypes::CopyToArray<KeyTree*>(const void* item, void* arrpointer)
{
	memcpy(arrpointer, item, sizeof(KeyTree*));
}

template<> void standardtypes::CopyToArray<BTree*>(const void* item, void* arrpointer)
{
	memcpy(arrpointer, item, sizeof(BTree*));
}

fs::wstring KeyTreeString(const void* item)
{
	wstringstream result;
	result << (*(KeyTree**)item)->ToString();
	return result.str();
}

ScalarType GetKeyTreeType()
{
	ScalarType type;
	type.CopyIn = CopyToArray<KeyTree*>;
	type.Size = sizeof(KeyTree*);
	type.ToString = KeyTreeString;
	return type;
}

ScalarType GetBTreeType()
{
	ScalarType type;
	type.CopyIn = CopyToArray<BTree*>;
	type.Size = sizeof(BTree*);
	return type;
}


class TreeBuffer : public IColumnBuffer
{
	public:
		TreeBuffer(const int& columnId, const ScalarType& rowType, const ScalarType &valueType, const fs::wstring& name);
		ValueVector GetValues(const KeyVector& rowId);
		bool Include(Value value, Key rowId);
		bool Exclude(Value value, Key rowId);
		bool Exclude(Key rowId);

		GetResult GetRows(Range& range);
		ValueKeysVectorVector GetSorted(const KeyVectorVector& input);
		Statistics GetStatistics();

		ScalarType GetRowIDType();
		ScalarType GetValueType();
		fs::wstring GetName();
		bool GetUnique();
		bool GetRequired();
		int GetID();

		Value GetValue(Key rowId);
		fs::wstring ToString();

	private:
		void ValuesMoved(void*, Node*);
		
		ValueKeysVector BuildData(BTree::iterator&, BTree::iterator&, void*, bool, int, bool&);
		ScalarType _rowType;
		ScalarType _valueType;
		ScalarType _nodeType;
		BTree* _rows;
		BTree* _values;
		fs::wstring _name;
		bool _required;
		long long _unique;
		long long _total;
		int _id;
};

inline TreeBuffer::TreeBuffer(const int& columnId, const ScalarType& rowType, const ScalarType &valueType, const fs::wstring& name)
{
	_id = columnId;
	_name = name;
	_rowType = rowType;
	_valueType = valueType;
	_nodeType = GetNodeType();
	_rows = new BTree(_rowType, _nodeType);
	_values = new BTree(_valueType, GetKeyTreeType());
	_required = false;
	_unique = 0;
	_total = 0;
	_values->setValuesMovedCallback
	(
		[this](void* value, Node* newLeaf) -> void
		{
			this->ValuesMoved(value, newLeaf);
		}
	);
}

inline int TreeBuffer::GetID()
{
	return _id;
}

inline Statistics TreeBuffer::GetStatistics()
{
	return Statistics(_total, _unique);
}

inline fs::wstring TreeBuffer::ToString()
{
	return _values->ToString();
}

inline fs::wstring TreeBuffer::GetName()
{
	return _name;
}

inline ScalarType TreeBuffer::GetValueType()
{
	return _valueType;
}

inline ScalarType TreeBuffer::GetRowIDType()
{
	return _rowType;
}

inline bool TreeBuffer::GetUnique()
{
	return false;
}

inline bool TreeBuffer::GetRequired()
{
	return _required;
}

inline ValueVector TreeBuffer::GetValues(const KeyVector& rowIds)
{
	ValueVector values(rowIds.size());
	for (unsigned int i = 0; i < rowIds.size(); i++)
	{
		values[i] = GetValue(rowIds[i]);
	}

	return values;
}

inline Value TreeBuffer::GetValue(Key rowId)
{
	bool match;
	auto iterator = _rows->find(rowId, match);
	
	if (match)
	{
		Node* leaf = *(Node**)(*(iterator)).value;
		return leaf->GetKey
		(
			[rowId](void* kt) -> bool
			{
				return (*(KeyTree**)kt)->GetPath(rowId).Match;
			}
		);
	}
	else
	{
		return NULL;
	}
}

inline ValueKeysVectorVector TreeBuffer::GetSorted(const KeyVectorVector& input)
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

inline bool TreeBuffer::Include(Value value, Key rowId)
{
	//TODO: Return Undo Information
	BTree::Path  path = _values->GetPath(value);
	if (path.Match)
	{
		KeyTree* existing = *(KeyTree**)(*path.Leaf)[path.LeafIndex].value;
		
		auto keypath = existing->GetPath(rowId);
		if (!keypath.Match)
		{
			existing->Insert(keypath, rowId);

			auto rowpath = _rows->GetPath(rowId);

			_rows->Insert(rowpath, rowId, &path.Leaf);

			_total++;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		KeyTree* newRows = new KeyTree(_rowType);

		auto keypath = newRows->GetPath(rowId);
		newRows->Insert(keypath, rowId);
		auto rowpath = _rows->GetPath(rowId);
		_rows->Insert(rowpath, rowId, &path.Leaf);		
		//Insert may generate a different leaf that the value gets inserted into,
		//so the above may be incorrect momentarily. If the value gets inserted
		//on a new split, the callback will be run and change the entry.
		_values->Insert(path, value, &newRows);

		_unique++;
		_total++;
		return true;
	}
}

inline bool TreeBuffer::Exclude(Value value, Key rowId)
{
	BTree::Path  path = _values->GetPath(value);
	//If existing is NULL, that row id did not exist under that value
	if (path.Match)
	{
		KeyTree* existing = *(KeyTree**)(*path.Leaf)[path.LeafIndex].value;
		auto keypath = existing->GetPath(rowId);
		if (keypath.Match)
		{
			existing->Delete(keypath);
			if (existing->Count() == 0)
			{
				_values->Delete(path);
				delete(existing);
				_unique--;
			}
			auto rowpath = _rows->GetPath(rowId);
			if (rowpath.Match)
				_rows->Delete(rowpath);

			_total--;
			return true;
		}
		else
		{
			return false;
		}
	}
	
	return false;
}

inline bool TreeBuffer::Exclude(Key rowId)
{
	Value val = GetValue(rowId);
	return Exclude(val, rowId);
}

inline void TreeBuffer::ValuesMoved(void* value, Node* leaf)
{
	KeyTree* existingValues = *(KeyTree**)(value);

	auto start = existingValues->begin();
	auto end = existingValues->end();

	while (!start.End())
	{
		//TODO: Make Btree or iterator writeable
		auto result = _rows->GetPath((*start).key);
		
		if (result.Match)
		{
			_nodeType.CopyIn(&leaf, (*result.Leaf)[result.LeafIndex].value);
		}
		else
		{
			throw;
		}

		++start;
	}	
}

inline GetResult TreeBuffer::GetRows(Range& range)
{
	//These may not exist, add logic for handling that.
	GetResult result;

	bool firstMatch = false; //Seeking to beginning or end
	bool lastMatch = false;
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
			range.Limit, 
			result.Limited
		);

	return result;
}

inline ValueKeysVector TreeBuffer::BuildData(BTree::iterator& first, BTree::iterator& last, Key startId, bool ascending, int limit, bool &limited)
{	
	int num = 0;
	bool foundCurrentId = startId == NULL;
    limited = false;
	ValueKeysVector rows;	
	
	while (first != last && !limited)
	{
		auto rowIds = (KeyTree*)*(void**)((*first).value);
		
		auto idStart = rowIds->begin();
		auto idEnd = rowIds->end();

		//Assumption.. The Id will exist in the first value we pull
		//Otherwise either our tree has changed (we are on the wrong revision) or we have an invalid start id, and this loop will never terminate. Ever. Until the end of time.
		while (!foundCurrentId)
		{
			if ((*idStart).key == startId)
				foundCurrentId = true;

			idStart++;				
		}

		KeyVector keys;
		while (idStart != idEnd)
		{
			keys.push_back((*idStart).key);
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
