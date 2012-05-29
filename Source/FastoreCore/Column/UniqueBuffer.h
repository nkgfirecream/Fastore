#pragma once

#include "..\typedefs.h"
#include "..\Schema\standardtypes.h"
#include "..\BTree.h"
#include "..\Column\IColumnBuffer.h"

const int UniqueBufferRowMapInitialSize = 32;

class UniqueBuffer : public IColumnBuffer
{
	public:
		UniqueBuffer(const ScalarType& rowType, const ScalarType& valueType);
		ValueVector GetValues(const KeyVector& rowId);
		bool Include(Value value, Key rowId);
		bool Exclude(Value value, Key rowId);
		bool Exclude(Key rowId);

		GetResult GetRows(Range& range);
		ValueKeysVectorVector GetSorted(const KeyVectorVector& input);
		Statistics GetStatistics();

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
};

inline UniqueBuffer::UniqueBuffer(const ScalarType& rowType, const ScalarType& valueType)
{
	_rowType = rowType;
	_valueType = valueType;
	_nodeType = GetNodeType();
	_rows = new BTree(_rowType, _nodeType);
	_values = new BTree(_valueType, standardtypes::StandardHashSet);
	_count = 0;
	_values->setValuesMovedCallback
	(
		[this](void* value, Node* newLeaf) -> void
		{
			this->ValuesMoved(value, newLeaf);
		}
	);
}

inline Statistics UniqueBuffer::GetStatistics()
{
	return Statistics(_count, _count);
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
	auto iterator = _rows->findNearest(rowId, match);
	
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
		BTree valueKeyTree(_valueType, standardtypes::StandardKeyVector);
		
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
	if (range.Start.HasValue() && range.End.HasValue())
	{		
		if (_valueType.Compare((*(range.Start)).Value, (*(range.End)).Value) > 0)
		{
			//Start is after end-> Invalid input.
			throw "Invalid range. Start is after end";
		}
	}

	//cache end marker since we will use it several times
	BTree::iterator lastValue = _values->end();
	
	bool beginMatch = false;
	bool endMatch = false;	

	BTree::iterator begin = range.Start.HasValue() ? _values->findNearest((*range.Start).Value, beginMatch) : _values->begin();
	BTree::iterator end =  range.End.HasValue() ? _values->findNearest((*range.End).Value, endMatch) : lastValue;

	bool bInclusive = range.Start.HasValue() ? (*(range.Start)).Inclusive : true;
	bool eInclusive = range.End.HasValue() ? (*(range.End)).Inclusive : true;

	GetResult result;	

	//Nothing in this range, so return empty result
	if ((begin == lastValue) || ((begin == end) && (!bInclusive || !eInclusive)))
		return result;	

	if (!bInclusive && beginMatch)
		begin++;

	if (eInclusive && endMatch)
		end++;

	result.Data = 
		BuildData
		(
			begin, 
			end,
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

	if (ascending)
	{
		while (first != last && !limited)
		{
			auto rowId = (Key)((*first).value);
		
			KeyVector keys;
			keys.push_back(rowId);			
			rows.push_back(ValueKeys((*first).key, keys));
			
			num++;
			limited = num == limit;

			first++;
		}
	}
	else
	{
		while (first != last && !limited)
		{
			last--;
			auto rowId = (Key)((*last).value);
		
			KeyVector keys;
			keys.push_back(rowId);
			rows.push_back(ValueKeys((*last).key, keys));

			num++;
			limited = num == limit;
		}
	}	

	return rows;
}
