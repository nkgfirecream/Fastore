#pragma once

#include "..\typedefs.h"
#include "..\Schema\standardtypes.h"
#include "..\BTree.h"
#include "..\Column\IColumnBuffer.h"

const int UniqueBufferRowMapInitialSize = 32;

class UniqueBuffer : public IColumnBuffer
{
	public:
		UniqueBuffer(const int& columnId, const ScalarType& rowType, const ScalarType& valueType, const fs::string& name);
		ValueVector GetValues(const KeyVector& rowId);
		bool Include(Value value, Key rowId);
		bool Exclude(Value value, Key rowId);
		bool Exclude(Key rowId);

		GetResult GetRows(Range& range);
		ValueKeysVectorVector GetSorted(const KeyVectorVector& input);
		Statistics GetStatistics();

		ScalarType GetRowIDType();
		ScalarType GetValueType();
		fs::string GetName();
		bool GetUnique();
		bool GetRequired();
		int GetID();

	private:
		void ValuesMoved(void*, Node*);
		Value GetValue(Key rowId);
		ValueKeysVector BuildData(BTree::iterator&, BTree::iterator&, bool, int, bool&);
		ScalarType _rowType;
		ScalarType _valueType;
		ScalarType _nodeType;
		BTree* _rows;
		BTree* _values;
		long long _count;
		fs::string _name;
		bool _required;
		int _id;
};

inline UniqueBuffer::UniqueBuffer(const int& columnId, const ScalarType& rowType, const ScalarType& valueType, const fs::string& name)
{
	_id = columnId;
	_name = name;
	_rowType = rowType;
	_valueType = valueType;
	_nodeType = GetNodeType();
	_rows = new BTree(_rowType, _nodeType);
	_values = new BTree(_valueType, standardtypes::GetHashSetType());
	_required = false;
	_count = 0;
	_values->setValuesMovedCallback
	(
		[this](void* value, Node* newLeaf) -> void
		{
			this->ValuesMoved(value, newLeaf);
		}
	);
}

inline int UniqueBuffer::GetID()
{
	return _id;
}

inline Statistics UniqueBuffer::GetStatistics()
{
	return Statistics(_count, _count);
}

inline fs::string UniqueBuffer::GetName()
{
	return _name;
}

inline ScalarType UniqueBuffer::GetValueType()
{
	return _valueType;
}

inline ScalarType UniqueBuffer::GetRowIDType()
{
	return _rowType;
}

inline bool UniqueBuffer::GetUnique()
{
	return true;
}

inline bool UniqueBuffer::GetRequired()
{
	return _required;
}


inline ValueVector UniqueBuffer::GetValues(const KeyVector& rowIds)
{
	ValueVector values(rowIds.size());
	for (unsigned int i = 0; i < rowIds.size(); i++)
	{
		values[i] = GetValue(rowIds[i]);
	}

	return values;
}

inline Value UniqueBuffer::GetValue(Key rowId)
{
	bool match;
	auto iterator = _rows->find(rowId, match);
	
	if (match)
	{
		Node* leaf = *(Node**)(*(iterator)).value;
		auto locRowType = _rowType;
		return leaf->GetKey
		(
			// TODO: would it help perf to put this callback into the type?
			[rowId, locRowType](void* foundId) -> bool
			{
				return locRowType.Compare(foundId, rowId) == 0;
			}
		);
	}
	else
	{
		return NULL;
	}
}

inline ValueKeysVectorVector UniqueBuffer::GetSorted(const KeyVectorVector& input)
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

inline bool UniqueBuffer::Include(Value value, Key rowId)
{
	//TODO: Return Undo Information
	BTree::Path path = _values->GetPath(value);
	if (path.Match)
		return false;
	else
	{
		auto rowpath = _rows->GetPath(rowId);
		_rows->Insert(rowpath, rowId, &path.Leaf);
		//Insert may generate a different leaf that the value gets inserted into,
		//so the above may be incorrect momentarily. If the value gets inserted
		//on a new split, the callback will be run and change the entry.
		_values->Insert(path, value, rowId);
		_count++;
		return true;
	}
}

inline bool UniqueBuffer::Exclude(Value value, Key rowId)
{
	BTree::Path  path = _values->GetPath(value);
	//If existing is NULL, that row id did not exist under that value
	if (path.Match)
	{
		Key existing = (Key)(*path.Leaf)[path.LeafIndex].value;
		if (_rowType.Compare(existing, rowId) == 0)
		{
			_values->Delete(path);
			auto rowpath = _rows->GetPath(rowId);
			_rows->Delete(rowpath);
			_count--;
			return true;
		}
	}
	
	return false;
}

inline bool UniqueBuffer::Exclude(Key rowId)
{
	Value val = GetValue(rowId);
	return Exclude(val, rowId);
}

inline void UniqueBuffer::ValuesMoved(Key value, Node* leaf)
{
	auto result = _rows->GetPath(value);
	if (result.Match)
	{
		_nodeType.CopyIn(&leaf, (*result.Leaf)[result.LeafIndex].value);
	}
}

inline GetResult UniqueBuffer::GetRows(Range& range)
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
			range.Ascending, 
			range.Limit, 
			result.Limited
		);

	return result;
}

inline ValueKeysVector UniqueBuffer::BuildData(BTree::iterator& first, BTree::iterator& last, bool ascending, int limit, bool &limited)
{	
	int num = 0;
    limited = false;
	ValueKeysVector rows;	
	
	while (first != last && !limited)
	{
		auto rowId = (Key)((*first).value);
		
		KeyVector keys;
		keys.push_back(rowId);
		num++;
		if (num == limit)
		{
			limited = true; break;
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
