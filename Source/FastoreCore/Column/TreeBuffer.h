#pragma once

#include "..\typedefs.h"
#include "..\Schema\standardtypes.h"
#include "..\BTree.h"
#include "..\KeyTree.h"
#include "..\Column\IColumnBuffer.h"

class TreeBuffer : public IColumnBuffer
{
	public:
		TreeBuffer(const ScalarType& rowType, const ScalarType &valueType);
		ValueVector GetValues(const KeyVector& rowId);
		bool Include(Value value, Key rowId);
		bool Exclude(Value value, Key rowId);
		bool Exclude(Key rowId);

		GetResult GetRows(Range& range);
		ValueKeysVectorVector GetSorted(const KeyVectorVector& input);
		Statistics GetStatistics();

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
		long long _unique;
		long long _total;

};

inline TreeBuffer::TreeBuffer(const ScalarType& rowType, const ScalarType &valueType)
{
	_rowType = rowType;
	_valueType = valueType;
	_nodeType = GetNodeType();
	_rows = new BTree(_rowType, _nodeType);
	_values = new BTree(_valueType, standardtypes::StandardKeyTree);
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

inline Statistics TreeBuffer::GetStatistics()
{
	return Statistics(_total, _unique);
}

inline ValueVector TreeBuffer::GetValues(const fs::KeyVector& rowIds)
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
	auto iterator = _rows->findNearest(rowId, match);
	
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

	while (start != end)
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

	int num = 0;
	if (range.Ascending)
	{
		while (begin != end && !result.Limited)
		{
			auto rowIds = (KeyTree*)*(void**)((*begin).value);			
			auto key = (Key)((*begin).key);

			auto idStart = rowIds->begin();
			auto idEnd = rowIds->end();

			while (!startFound)
			{
				if (_rowType.Compare((*idStart).key, startId) == 0)
				{
					startFound = true;
					result.BeginOfFile = false;
				}

				++idStart;				
			}
		
			KeyVector keys;
			while (idStart != idEnd && !result.Limited)
			{
				keys.push_back((*idStart).key);
				++num;
				++idStart;

				result.Limited = num == range.Limit;
			}

			if (keys.size() > 0)
				result.Data.push_back(ValueKeys(key, keys));

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
			auto rowIds = (KeyTree*)*(void**)((*end).value);		
			auto key = (Key)((*end).key);

			auto idStart = rowIds->begin();
			auto idEnd = rowIds->end();

			while (!startFound)
			{
				--idEnd;

				if (_rowType.Compare((*idEnd).key, startId) == 0)
				{
					startFound = true;
					result.EndOfFile = false;
				}			
			}
		
			KeyVector keys;
			while (idStart != idEnd && !result.Limited)
			{	
				--idEnd;
				keys.push_back((*idEnd).key);
				++num;

				result.Limited = num == range.Limit;	
			}
		
			if (keys.size() > 0)
				result.Data.push_back(ValueKeys(key, keys));
		}

		//if we didn't make it through the entire set, reset the bof marker.
		if (result.Limited && result.BeginOfFile)
			result.BeginOfFile = false;
	}

	return result;
}
