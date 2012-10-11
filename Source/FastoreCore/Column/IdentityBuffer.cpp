#include "IdentityBuffer.h"

IdentityBuffer::IdentityBuffer(const ScalarType& type)
{
	_type = type;
	_rows = std::unique_ptr<BTree>(new BTree(_type));
	_count = 0;
}

Statistic IdentityBuffer::GetStatistic()
{
	Statistic stat;
	stat.total = _count;
	stat.unique = _count;
	return stat;
}

vector<OptionalValue> IdentityBuffer::GetValues(const vector<std::string>& rowIds)
{
	vector<OptionalValue> values(rowIds.size());
	for (unsigned int i = 0; i < rowIds.size(); i++)
	{
		auto path = _rows->GetPath(_type.GetPointer(rowIds[i]));
		if (path.Match)
		{
			values[i].__set_value(rowIds[i]);
		}
	}

	return values;
}

void IdentityBuffer::Apply(const ColumnWrites& writes)
{
	auto exstart = writes.excludes.begin();
	while (exstart != writes.excludes.end())
	{
		void* rowId = _type.GetPointer((*exstart).rowID);
		Exclude(rowId);
		exstart++;
	}

	auto incstart = writes.includes.begin();
	while (incstart != writes.includes.end())
	{
		void* value = _type.GetPointer((*incstart).value);
		void* rowId = _type.GetPointer((*incstart).rowID);

		if (_type.Compare(value,rowId) != 0)
			throw "Identity buffer requires the rowId and value to be the same";

		Include(rowId);
		incstart++;
	}
}

bool IdentityBuffer::Include(void* rowId)
{
	//TODO: Return Undo Information
	auto rowpath = _rows->GetPath(rowId);
	if (rowpath.Match)
		return false;
	else
	{		
		_rows->Insert(rowpath, rowId, rowId);
		_count++;
		return true;
	}
}

bool IdentityBuffer::Exclude(void* rowId)
{
	auto rowpath = _rows->GetPath(rowId);
	if (!rowpath.Match)
		return false;
	else
	{
		_rows->Delete(rowpath);
		_count--;
		return true;
	}
}


RangeResult IdentityBuffer::GetRows(const RangeRequest& range)
{
	void* firstp = range.__isset.first ? _type.GetPointer(range.first.value) : NULL;
	void* lastp = range.__isset.last ? _type.GetPointer(range.last.value) : NULL;
	void* startId = range.__isset.rowID ? _type.GetPointer(range.rowID) : NULL;

	RangeResult result;	
	ValueRowsList vrl;

	if (range.__isset.first && range.__isset.last)
	{		
		if ((range.ascending && _type.Compare(firstp, lastp) > 0) || (!range.ascending && _type.Compare(firstp, lastp) < 0))
		{
			//Disjoint constraints - (e.g. < 2 && > 5). Return empty set.
			result.__set_valueRowsList(vrl);
			result.__set_bof(true);
			result.__set_eof(true);
			result.__set_limited(false);
		}
	}

	//cache markers since we will use it several times
	BTree::iterator lastMarker = range.ascending ? _rows->end() : _rows->begin();
	BTree::iterator firstMarker = range.ascending ? _rows->begin() : _rows->end();

	bool bInclusive = range.__isset.first ? range.first.inclusive : true;
	bool eInclusive = range.__isset.last ? range.last.inclusive : true;

	bool beginMatch = false;
	bool endMatch = false;	
		
	BTree::iterator begin = 
	startId != NULL  ? _rows->findNearest(startId, beginMatch) :
	range.__isset.first ? _rows->findNearest(firstp, beginMatch) :
	firstMarker;

	BTree::iterator end = 
	range.__isset.last ? _rows->findNearest(lastp, endMatch) :
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
		if (!range.ascending)
			--begin;

		auto rowId = (void*)((*begin).key);

		if (!startFound && _type.Compare(rowId, startId) == 0)
		{				
			result.__set_bof(false);
			startFound = true;
			if (range.ascending)
				++begin;

			continue;
		}
			
		ValueRows vr;
		string value;
		_type.CopyOut(rowId, value);
		vr.__set_value(value);			
			
		string rowIdcopy;
		_type.CopyOut(rowId, rowIdcopy);

		std::vector<string> rowIds;
		rowIds.push_back(rowIdcopy);

		vr.__set_rowIDs(rowIds);
		vrl.push_back(vr);
		result.__set_limited(vrl.size() == SAFE_CAST(size_t, range.limit));

		if (range.ascending)
				++begin;
	}

	//if we didn't make it through the entire set, reset the eof marker.
	if (result.limited && result.eof)
		result.__set_eof(false);

	result.__set_valueRowsList(vrl);
	return result;
}
