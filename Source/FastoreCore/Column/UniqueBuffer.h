#pragma once

#include "..\Schema\standardtypes.h"
#include "..\BTree.h"
#include "..\Column\IColumnBuffer.h"

const int UniqueBufferRowMapInitialSize = 32;

class UniqueBuffer : public IColumnBuffer
{
	public:
		UniqueBuffer(const ScalarType& rowType, const ScalarType& valueType);

		vector<std::string> GetValues(const vector<std::string>& rowIds);		
		void Apply(const ColumnWrites& writes);
		RangeResult GetRows(const RangeRequest& range);
		Statistic GetStatistic();

	private:
		bool Include(void* rowId, void* value);
		bool Exclude(void* rowId);
		bool Exclude(void* rowId, void* value);
		void* GetValue(void* rowId);

		void ValuesMoved(void* value, Node* leaf);		
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
	_nodeType = NoOpNodeType();
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

inline Statistic UniqueBuffer::GetStatistic()
{
	Statistic stat;
	stat.total = _count;
	stat.unique = _count;
	return stat;
}

inline vector<std::string> UniqueBuffer::GetValues(const vector<std::string>& rowIds)
{
	vector<std::string> values(rowIds.size());
	for (unsigned int i = 0; i < rowIds.size(); i++)
	{
		 _valueType.CopyOut(GetValue(_valueType.GetPointer(rowIds[i])),values[i]);
	}

	return values;
}

inline void* UniqueBuffer::GetValue(void* rowId)
{
	bool match;
	auto iterator = _rows->findNearest(rowId, match);
	
	if (match)
	{
		Node* leaf = *(Node**)(*(iterator)).value;
		auto locRowType = _rowType;

		void* result = leaf->GetKey
		(
			[rowId, locRowType](void* foundId) -> bool
			{
				return locRowType.Compare(rowId, foundId) == 0;
			}
		);
	}
	else
	{
		return NULL;
	}
}

inline void UniqueBuffer::Apply(const ColumnWrites& writes)
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

inline bool UniqueBuffer::Include(void* rowId, void* value)
{
	//TODO: Return Undo Information
	auto rowpath = _rows->GetPath(rowId);
	if (rowpath.Match)
		return false;

	BTree::Path path = _values->GetPath(value);
	if (path.Match)
		return false;
	else
	{
		
		_rows->Insert(rowpath, rowId, &path.Leaf);
		//Insert may generate a different leaf that the value gets inserted into,
		//so the above may be incorrect momentarily. If the value gets inserted
		//on a new split, the callback will be run and change the entry.
		_values->Insert(path, value, rowId);
		_count++;
		return true;
	}
}

inline bool UniqueBuffer::Exclude(void* rowId, void* value)
{
	auto rowpath = _rows->GetPath(rowId);
	if (!rowpath.Match)
		return false;

	BTree::Path  path = _values->GetPath(value);
	if (path.Match)
	{
		void* existing = (void*)(*path.Leaf)[path.LeafIndex].value;
		if (_rowType.Compare(rowId, existing) == 0)
		{
			_values->Delete(path);
			_rows->Delete(rowpath);
			_count--;
			return true;
		}
	}
	
	return false;
}

inline bool UniqueBuffer::Exclude(void* rowId)
{
	void* val = GetValue(rowId);
	return Exclude(rowId, val);
}

inline void UniqueBuffer::ValuesMoved(void* value, Node* leaf)
{
	auto result = _rows->GetPath(value);
	if (result.Match)
	{
		_nodeType.CopyIn(&leaf, (*result.Leaf)[result.LeafIndex].value);
	}
}

inline RangeResult UniqueBuffer::GetRows(const RangeRequest& range)
{
	//TODO: Get this from the query..
	int limit = 500;

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

	result.beginOfRange = begin == firstMarker;
	result.endOfRange = end == lastMarker;

	//Nothing in this range, so return empty result
	if ((begin == lastMarker) || ((begin == end) && (!bInclusive || !eInclusive)))
		return result;	

	if (!bInclusive && beginMatch)
	{
		//reset BOF Marker since we are excluding
		result.beginOfRange = false;
		++begin;
	}

	if (eInclusive && endMatch)
	{
		++end;
		//reset EOF Marker since we are including
		result.endOfRange = end == lastMarker;
	}	

	bool startFound = startId == NULL;	

	ValueRowsList vrl;

	if (range.ascending)
	{		
		while (begin != end && !result.limited)
		{
			auto rowId = (void*)((*begin).value);
			auto key = (void*)((*begin).key);

			if (!startFound && _rowType.Compare(rowId, startId) == 0)
			{				
				result.beginOfRange = false;
				startFound = true;
				begin++;
				continue;
			}
			
			ValueRows vr;
			string value;
			_valueType.CopyOut(key, value);
			vr.__set_value(value);			
			
			string rowIdcopy;
			_rowType.CopyOut(rowId, rowIdcopy);

			std::vector<string> rowIds;
			rowIds.push_back(rowIdcopy);

			vr.__set_rowIDs(rowIds);
			vrl.push_back(vr);
			result.limited = vrl.size() == limit;

			++begin;
		}

		//if we didn't make it through the entire set, reset the eof marker.
		if (result.limited && result.endOfRange)
			result.endOfRange = false;
	}
	else
	{
		while (begin != end && !result.limited)
		{
			--end;

			auto rowId = (void*)((*end).value);
			auto key = (void*)((*end).key);

			if (!startFound && _rowType.Compare(rowId, startId) == 0)
			{
				result.endOfRange = false;
				startFound = true;
				continue;
			}
		
			ValueRows vr;
			string value;
			_valueType.CopyOut(key, value);
			vr.__set_value(value);			
			
			string rowIdcopy;
			_rowType.CopyOut(rowId, rowIdcopy);

			std::vector<string> rowIds;
			rowIds.push_back(rowIdcopy);

			vr.__set_rowIDs(rowIds);
			vrl.push_back(vr);
			result.limited = vrl.size() == limit;

			++begin;
		}

		//if we didn't make it through the entire set, reset the bof marker.
		if (result.limited && result.beginOfRange)
			result.beginOfRange = false;
	}	

	result.__set_valueRowsList(vrl);

	return result;
}
