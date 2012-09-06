#pragma once

#include "../Schema/standardtypes.h"
#include "../BTree.h"
#include "../Column/IColumnBuffer.h"

const int UniqueBufferRowMapInitialSize = 32;

#define SAFE_CAST(t,f) safe_cast(__FILE__, __LINE__, (t), (f))

template <typename T, typename F>
T safe_cast(const char file[], size_t line, T, F input) {
  using std::numeric_limits;
  std::ostringstream msg;

  if( numeric_limits<F>::is_signed && !numeric_limits<T>::is_signed ) {
    if( input < 0 ) {
      msg << file << ":" << line << ": " 
	  << "signed value " << input << " cannot be cast to unsigned type";
      throw std::runtime_error(msg.str());
    }
    if( numeric_limits<T>::max() < static_cast<size_t>(input) ) {
      msg << file << ":" << line << ": " 
	  << input << ", size " << sizeof(F) 
	  << ", cannot be cast to unsigned type of size" << sizeof(T);
      throw std::runtime_error(msg.str());
    }
  }
  return static_cast<T>(input);
}

class UniqueBuffer : public IColumnBuffer
{
	public:
		UniqueBuffer(const ScalarType& rowType, const ScalarType& valueType);

		vector<OptionalValue> GetValues(const vector<std::string>& rowIds);		
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

inline vector<OptionalValue> UniqueBuffer::GetValues(const vector<std::string>& rowIds)
{
	vector<OptionalValue> values(rowIds.size());
	for (unsigned int i = 0; i < rowIds.size(); i++)
	{
		auto result = GetValue(_rowType.GetPointer(rowIds[i]));
		if (result != NULL)
		{
			std::string value;
			_valueType.CopyOut(result, value);
			values[i].__set_value(value);
		}
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

		return leaf->GetKey
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
	void* firstp = range.__isset.first ? _valueType.GetPointer(range.first.value) : NULL;
	void* lastp = range.__isset.last ? _valueType.GetPointer(range.last.value) : NULL;
	void* startId = range.__isset.rowID ? _rowType.GetPointer(range.rowID) : NULL;

	if (range.__isset.first && range.__isset.last)
	{		
		if (range.ascending && _valueType.Compare(firstp, lastp) > 0)
		{
			throw "Invalid range. Start is after end and the range is ascending";
		}
		else if (!range.ascending && _valueType.Compare(firstp, lastp) < 0)
		{
			throw "Invalid range. Start is after end and the range is descending";
		}
	}

	//cache markers since we will use it several times
	BTree::iterator lastMarker = range.ascending ? _values->end() : _values->begin();
	BTree::iterator firstMarker = range.ascending ? _values->begin() : _values->end();

	bool bInclusive = range.__isset.first ? range.first.inclusive : true;
	bool eInclusive = range.__isset.last ? range.last.inclusive : true;

	bool beginMatch = false;
	bool endMatch = false;	
		
	BTree::iterator begin = 
	startId != NULL  ? _values->findNearest(GetValue(startId), beginMatch) :
	range.__isset.first ? _values->findNearest(firstp, beginMatch) :
	firstMarker;

	BTree::iterator end = 
	range.__isset.last ? _values->findNearest(lastp, endMatch) :
	lastMarker;

	RangeResult result;	
	ValueRowsList vrl;

	result.__set_bof(begin == firstMarker);
	result.__set_eof(end == lastMarker);

	bool startFound = startId == NULL;

	//Set up bounds to point to correct values, setup lambdas to move iterator (and avoid branches/cleanup code)
	//TODO : Figure out cost of branches vs lambdas... Could make a lot of use of them in the Tree code...
	if (range.ascending)
	{
		//ascending iterates AFTER grabbing value.
		if (!bInclusive && beginMatch)
		{
			++begin;
			//reset BOF Marker since we are excluding
			result.__set_bof(false);
		}

		if (eInclusive && endMatch)
		{
			++end;
			//reset EOF Marker since we are including
			result.__set_eof(end == lastMarker);
		}
	}
	else
	{
		//descending iterates BEFORE grabbing value...
		if (bInclusive && beginMatch)
		{
			++begin;
			//reset BOF Marker since we are excluding
			result.__set_bof(false);
		}

		if (!eInclusive && endMatch)
		{
			++end;
			//reset EOF Marker since we are including
			result.__set_eof(end == lastMarker);
		}
	}

	while (begin != end && !result.limited)
	{
		if(!range.ascending)
			--begin;

		auto valuep = (void*)((*begin).key);
		auto rowIdp = (void*)((*begin).value);

		if (!startFound && _rowType.Compare(rowIdp, startId) == 0)
		{				
			result.__set_bof(false);
			startFound = true;
			if(range.ascending)
				++begin;
			continue;
		}
			
		ValueRows vr;
		string value;
		_valueType.CopyOut(valuep, value);
		vr.__set_value(value);			
			
		string rowIdcopy;
		_rowType.CopyOut(rowIdp, rowIdcopy);

		std::vector<string> rowIds;
		rowIds.push_back(rowIdcopy);

		vr.__set_rowIDs(rowIds);
		vrl.push_back(vr);
		result.__set_limited(vrl.size() == SAFE_CAST(size_t(), range.limit));

		if(range.ascending)
				++begin;
	}

	//if we didn't make it through the entire set, reset the eof marker.
	if (result.limited && result.eof)
		result.__set_eof(false);

	result.__set_valueRowsList(vrl);
	return result;
}
