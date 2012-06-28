#pragma once

#include "..\Schema\standardtypes.h"
#include "..\BTree.h"
#include "..\KeyTree.h"
#include "..\Column\IColumnBuffer.h"

class TreeBuffer : public IColumnBuffer
{
	public:
		TreeBuffer(const ScalarType& rowType, const ScalarType &valueType);		

		vector<std::string> GetValues(const vector<std::string>& rowIds);		
		void Apply(const ColumnWrites& writes);
		RangeResult GetRows(const RangeRequest& range);
		Statistic GetStatistic();

	private:
		bool Include(void* rowId, void* value);
		bool Exclude(void* rowId);
		bool Exclude(void* rowId, void* value);
		void* GetValue(void* rowId);

		void ValuesMoved(void*, Node*);
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
	_nodeType = NoOpNodeType();
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

inline Statistic TreeBuffer::GetStatistic()
{
	Statistic stat;
	stat.total = _total;
	stat.unique = _unique;
	return stat;
}

inline vector<std::string> TreeBuffer::GetValues(const vector<std::string>& rowIds)
{
	vector<std::string> values(rowIds.size());
	for (unsigned int i = 0; i < rowIds.size(); i++)
	{
		 _valueType.CopyOut(GetValue(_rowType.GetPointer(rowIds[i])), values[i]);
	}

	return values;
}

inline void* TreeBuffer::GetValue(void* rowId)
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

inline void TreeBuffer::Apply(const ColumnWrites& writes)
{
	auto exstart = writes.excludes.begin();
	while (exstart != writes.excludes.end())
	{
		void* rowId = _rowType.GetPointer((*exstart).rowID);
		Exclude(rowId);
		exstart++;
	}

	auto incstart = writes.includes.begin();
	while (incstart != writes.includes.end())
	{
		void* value = _valueType.GetPointer((*incstart).value);
		void* rowId = _rowType.GetPointer((*incstart).rowID);
		Include(rowId, value);
		incstart++;
	}
}

inline bool TreeBuffer::Include(void* rowId, void* value)
{
	//TODO: Return Undo Information
	auto rowpath = _rows->GetPath(rowId);
	if (rowpath.Match)
		return false;

	BTree::Path  path = _values->GetPath(value);
	if (path.Match)
	{
		KeyTree* existing = *(KeyTree**)(*path.Leaf)[path.LeafIndex].value;
		
		auto keypath = existing->GetPath(rowId);
		existing->Insert(keypath, rowId);			

		_rows->Insert(rowpath, rowId, &path.Leaf);

		_total++;
		return true;
	}
	else
	{
		KeyTree* newRows = new KeyTree(_rowType);

		auto keypath = newRows->GetPath(rowId);

		newRows->Insert(keypath, rowId);
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

inline bool TreeBuffer::Exclude(void* value, void* rowId)
{
	auto rowpath = _rows->GetPath(rowId);
	if (!rowpath.Match)
		return false;

	BTree::Path  path = _values->GetPath(value);
	//If existing is NULL, that row id did not exist under that value
	if (path.Match)
	{
		KeyTree* existing = *(KeyTree**)(*path.Leaf)[path.LeafIndex].value;
		auto keypath = existing->GetPath(rowId);
		existing->Delete(keypath);
		if (existing->Count() == 0)
		{
			_values->Delete(path);
			_unique--;
		}
			
		_rows->Delete(rowpath);

		_total--;
		return true;
	}
	
	return false;
}

inline bool TreeBuffer::Exclude(void* rowId)
{
	void* val = GetValue(rowId);
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

inline RangeResult TreeBuffer::GetRows(const RangeRequest& range)
{
	void* firstp = range.__isset.first ? _valueType.GetPointer(range.first.value) : NULL;
	void* lastp = range.__isset.last ? _valueType.GetPointer(range.last.value) : NULL;
	void* startId = range.__isset.rowID ? _rowType.GetPointer(range.rowID) : NULL;

	if (range.__isset.first && range.__isset.last)
	{		
		if (_valueType.Compare(firstp, lastp) > 0)
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

	BTree::iterator begin = range.__isset.first ? _values->findNearest(firstp, beginMatch) : firstMarker;
	BTree::iterator end =  range.__isset.last ? _values->findNearest(lastp, endMatch) : lastMarker;

	bool bInclusive = range.__isset.first ? range.first.inclusive : true;
	bool eInclusive = range.__isset.last ? range.last.inclusive : true;

	RangeResult result;	

	result.__set_bof(begin == firstMarker);
	result.__set_eof(end == lastMarker);

	//Nothing in this range, so return empty result
	if ((begin == lastMarker) || ((begin == end) && (!bInclusive || !eInclusive)))
		return result;	

	if (!bInclusive && beginMatch)
	{
		//reset BOF Marker since we are excluding
		result.__set_bof(false);
		++begin;
	}

	if (eInclusive && endMatch)
	{
		++end;
		//reset EOF Marker since we are including
		result.__set_eof(end == lastMarker);
	}

	bool startFound = startId == NULL;
	int num = 0;
	ValueRowsList vrl;

	if (range.ascending)
	{
		while (begin != end && !result.limited)
		{
			auto rowIdTree = (KeyTree*)*(void**)((*begin).value);			
			auto key = (void*)((*begin).key);

			auto idStart = startFound ? rowIdTree->begin() : rowIdTree->find(startId);
			auto idEnd = rowIdTree->end();

			if (!startFound)
			{
				if (idStart != idEnd)
				{
					startFound = true;
					result.__set_bof(false);
					++idStart;			
				}
				else
				{
					throw "Start id not found in given value";
				}					
			}
		
			std::vector<std::string> rowIds;
			while (idStart != idEnd && !result.limited)
			{
				string rowId;
				_rowType.CopyOut((*idStart).key, rowId);
				rowIds.push_back(rowId);
				++num;
				++idStart;

				result.__set_limited(num == range.limit);
			}

			if (rowIds.size() > 0)
			{
				ValueRows vr;
				string value;
				_valueType.CopyOut(key, value);
				
				vr.__set_value(value);
				vr.__set_rowIDs(rowIds);
				vrl.push_back(vr);
			}

			++begin;
		}

		//if we didn't make it through the entire set, reset the eof marker.
		if (result.limited && result.eof)
			result.__set_eof(false);
	}
	else
	{
		while (begin != end && !result.limited)
		{
			--end;
			auto rowIdTree = (KeyTree*)*(void**)((*end).value);		
			auto key = (void*)((*end).key);

			auto idStart = rowIdTree->begin();
			auto idEnd = startFound ? rowIdTree->end() : rowIdTree->find(startId);

			if (!startFound)
			{
				if (idEnd != rowIdTree->end())
				{
					startFound = true;
					result.__set_eof(false);
				}
				else
				{
					throw "Start id not found in given value";
				}

				if (idEnd != idStart)
				--idEnd;
			}
		
			std::vector<std::string> rowIds;
			while (idStart != idEnd && !result.limited)
			{	
				--idEnd;
				string rowId;
				_rowType.CopyOut((*idEnd).key, rowId);
				rowIds.push_back(rowId);
				++num;

				result.__set_limited(num == range.limit);
			}
		
			if (rowIds.size() > 0)
			{
				ValueRows vr;
				string value;
				_valueType.CopyOut(key, value);
				
				vr.__set_value(value);
				vr.__set_rowIDs(rowIds);
				vrl.push_back(vr);
			}
		}

		//if we didn't make it through the entire set, reset the bof marker.
		if (result.limited && result.bof)
			result.__set_bof(false);
	}

	result.__set_valueRowsList(vrl);
	return result;
}
