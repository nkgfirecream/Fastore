#include "TreeInlineBuffer.h"

TreeInlineBuffer::TreeInlineBuffer(const ScalarType& rowType, const ScalarType &valueType) : _rowType(rowType), _valueType(valueType)
{
	_rows = std::unique_ptr<BTree>(new BTree(_rowType, _valueType));
	_values = std::unique_ptr<BTree>(new BTree(_valueType, standardtypes::StandardBTreeType));
	_unique = 0;
	_total = 0;
}

const ScalarType& TreeInlineBuffer::GetRowIdType()
{
	return _rowType;
}

const ScalarType& TreeInlineBuffer::GetValueType()
{
	return _valueType;
}

Statistic TreeInlineBuffer::GetStatistic()
{
	Statistic stat;
	stat.total = _total;
	stat.unique = _unique;
	return stat;
}

vector<OptionalValue> TreeInlineBuffer::GetValues(const vector<std::string>& rowIds)
{
	vector<OptionalValue> values(rowIds.size());
	for (unsigned int i = 0; i < rowIds.size(); ++i)
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

void* TreeInlineBuffer::GetValue(void* rowId)
{
	bool match;
	auto iterator = _rows->findNearest(rowId, match);
	
	if (match)
	{
		return (*(iterator)).value;
	}
	else
	{
		return NULL;
	}
}

void TreeInlineBuffer::Apply(const ColumnWrites& writes)
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

bool TreeInlineBuffer::Include(void* rowId, void* value)
{
	//TODO: Return Undo Information
	BTree::Path rowpath;
	_rows->GetPath(rowId, rowpath);
	if (rowpath.Match)
		return false;

	BTree::Path  path;
	_values->GetPath(value, path);
	if (path.Match)
	{
		BTree* existing = *(BTree**)(*path.Leaf)[path.LeafIndex].value;
		
		BTree::Path  keypath;
		existing->GetPath(rowId, keypath);
		existing->Insert(keypath, rowId, rowId);			

		_rows->Insert(rowpath, rowId, value);

		_total++;
		return true;
	}
	else
	{
		BTree* newRows = new BTree(_rowType);

		BTree::Path  keypath;
		newRows->GetPath(rowId, keypath);

		newRows->Insert(keypath, rowId, rowId);
		_rows->Insert(rowpath, rowId, value);		
		_values->Insert(path, value, &newRows);

		_unique++;
		_total++;
		return true;
	}
}

bool TreeInlineBuffer::Exclude(void* value, void* rowId)
{
	BTree::Path rowpath;
	_rows->GetPath(rowId, rowpath);
	if (!rowpath.Match)
		return false;

	BTree::Path  path;
	_values->GetPath(value, path);
	//If existing is NULL, that row id did not exist under that value
	if (path.Match)
	{
		BTree* existing = *(BTree**)(*path.Leaf)[path.LeafIndex].value;
		BTree::Path keypath;
		existing->GetPath(rowId, keypath);
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

bool TreeInlineBuffer::Exclude(void* rowId)
{
	void* val = GetValue(rowId);
	return Exclude(val, rowId);
}

RangeResult TreeInlineBuffer::GetRows(const RangeRequest& range)
{
	void* firstp = range.__isset.first ? _valueType.GetPointer(range.first.value) : NULL;
	void* lastp = range.__isset.last ? _valueType.GetPointer(range.last.value) : NULL;
	void* startId = range.__isset.rowID ? _rowType.GetPointer(range.rowID) : NULL;
	
	RangeResult result;	
	ValueRowsList vrl;

	if (range.__isset.first && range.__isset.last)
	{		
		if ((range.ascending && _valueType.Compare(firstp, lastp) > 0) || (!range.ascending && _valueType.Compare(firstp, lastp) < 0))
		{
			//Disjoint constraints - (e.g. < 2 && > 5). Return empty set.
			result.__set_valueRowsList(vrl);
			result.__set_bof(true);
			result.__set_eof(true);
			result.__set_limited(false);
		}
	}

	//cache markers since we will use it several times
	BTree::iterator firstMarker = range.ascending ? _values->begin() : _values->end();
	BTree::iterator lastMarker = range.ascending ? _values->end() : _values->begin();
	

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

	int num = 0;

	while (begin != end && !result.limited)
	{
		if (!range.ascending)
			--begin;

		auto rowIdTree = (BTree*)*(void**)((*begin).value);			
		auto key = (void*)((*begin).key);

		auto idStart = !startFound ?  rowIdTree->find(startId) :  range.ascending ? rowIdTree->begin() : rowIdTree->end();
		auto idEnd = range.ascending ? rowIdTree->end() : rowIdTree->begin();

		if (!startFound)
		{
			if (idStart != rowIdTree->end())
			{
				startFound = true;
				result.__set_bof(false);
				if (range.ascending)
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
			if (!range.ascending)
					--idStart;

			string rowId;
			_rowType.CopyOut((*idStart).key, rowId);
			rowIds.push_back(rowId);
			++num;

			if (range.ascending)
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

		if (range.ascending)
			++begin;
	}

	//if we didn't make it through the entire set, reset the eof marker.
	if (result.limited && result.eof)
		result.__set_eof(false);

	result.__set_valueRowsList(vrl);
	return result;
}
