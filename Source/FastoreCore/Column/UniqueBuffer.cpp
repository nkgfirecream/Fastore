#include "UniqueBuffer.h"

UniqueBuffer::UniqueBuffer(const ScalarType& rowType, const ScalarType& valueType): _rowType(rowType), _valueType(valueType)
{
	_rows = std::unique_ptr<BTree>(new BTree(_rowType, standardtypes::StandardNoOpNodeType));
	_values =  std::unique_ptr<BTree>(new BTree(_valueType, standardtypes::StandardHashSet));
	_count = 0;
	_values->setValuesMovedCallback
	(
		[this](void* value, Node* newLeaf) -> void
		{
			this->ValuesMoved(value, newLeaf);
		}
	);
}

Statistic UniqueBuffer::GetStatistic()
{
	Statistic stat;
	stat.total = _count;
	stat.unique = _count;
	return stat;
}

vector<OptionalValue> UniqueBuffer::GetValues(const vector<std::string>& rowIds)
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

void* UniqueBuffer::GetValue(void* rowId)
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

void UniqueBuffer::Apply(const ColumnWrites& writes)
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

bool UniqueBuffer::Include(void* rowId, void* value)
{
	//TODO: Return Undo Information
	BTree::Path rowpath;
	_rows->GetPath(rowId, rowpath);
	if (rowpath.Match)
		return false;

	BTree::Path path;
	_values->GetPath(value, path);
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

bool UniqueBuffer::Exclude(void* rowId, void* value)
{
	BTree::Path rowpath;
	_rows->GetPath(rowId, rowpath);
	if (!rowpath.Match)
		return false;

	BTree::Path  path;
	_values->GetPath(value, path);
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

bool UniqueBuffer::Exclude(void* rowId)
{
	void* val = GetValue(rowId);
	return Exclude(rowId, val);
}

void UniqueBuffer::ValuesMoved(void* value, Node* leaf)
{
	BTree::Path result;
	_rows->GetPath(value, result);
	if (result.Match)
	{
		standardtypes::StandardNodeType.CopyIn(&leaf, (*result.Leaf)[result.LeafIndex].value);
	}
}

RangeResult UniqueBuffer::GetRows(const RangeRequest& range)
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
		result.__set_limited(vrl.size() == SAFE_CAST(size_t, range.limit));

		if(range.ascending)
				++begin;
	}

	//if we didn't make it through the entire set, reset the eof marker.
	if (result.limited && result.eof)
		result.__set_eof(false);

	result.__set_valueRowsList(vrl);
	return result;
}
