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

	//cache markers since we will use it several times
	BTree::iterator lastMarker = _values->end();
	BTree::iterator firstMarker = _values->begin();
	
	bool beginMatch = false;
	bool endMatch = false;

	BTree::iterator begin = range.Start.HasValue() ? _values->findNearest((*range.Start).Value, beginMatch) : firstMarker;
	BTree::iterator end =  range.End.HasValue() ? _values->findNearest((*range.End).Value, endMatch) : lastMarker;

	bool bInclusive = range.Start.HasValue() ? (*(range.Start)).Inclusive : true;
	bool eInclusive = range.End.HasValue() ? (*(range.End)).Inclusive : true;

	GetResult result;	

	result.BeginOfFile = begin == firstMarker;
	result.EndOfFile = end == lastMarker;

	//Nothing in this range, so return empty result
	if ((begin == lastMarker) || ((begin == end) && (!bInclusive || !eInclusive)))
		return result;	

	if (!bInclusive && beginMatch)
	{
		//reset BOF Marker since we are excluding
		result.BeginOfFile = false;
		++begin;
	}

	if (eInclusive && endMatch)
	{
		++end;
		//reset EOF Marker since we are including
		result.EndOfFile = end == lastMarker;
	}

	void* startId = range.Start.HasValue() && (*(range.Start)).RowId.HasValue() ? *(*(range.Start)).RowId :
			range.End.HasValue() && (*(range.End)).RowId.HasValue() ? (*(*(range.End)).RowId) :
			NULL;

	bool startFound = startId == NULL;	

	if (range.Ascending)
	{		
		while (begin != end && !result.Limited)
		{
			auto rowId = (Key)((*begin).value);
			auto key = (Key)((*begin).key);

			if (!startFound && _rowType.Compare(rowId, startId) == 0)
			{				
				result.BeginOfFile = false;
				startFound = true;
				begin++;
				continue;
			}
		
			KeyVector keys;
			keys.push_back(rowId);			
			result.Data.push_back(ValueKeys(key, keys));			
			result.Limited = result.Data.size() == range.Limit;

			++begin;
		}

		//if we didn't make it through the entire set, reset the eof marker.
		if (result.Limited && result.EndOfFile)
			result.EndOfFile = false;
	}
	else
	{
		while (begin != end && !result.Limited)
		{
			--end;

			auto rowId = (Key)((*end).value);
			auto key = (Key)((*end).key);

			if (!startFound && _rowType.Compare(rowId, startId) == 0)
			{
				result.EndOfFile = false;
				startFound = true;
				continue;
			}
		
			KeyVector keys;
			keys.push_back(rowId);
			result.Data.push_back(ValueKeys(key, keys));
			result.Limited = result.Data.size() == range.Limit;
		}

		//if we didn't make it through the entire set, reset the bof marker.
		if (result.Limited && result.BeginOfFile)
			result.BeginOfFile = false;
	}
	

	return result;
}
