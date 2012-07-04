#pragma once

#include "..\Schema\standardtypes.h"
#include "..\KeyTree.h"
#include "..\Column\IColumnBuffer.h"

class IdentityBuffer : public IColumnBuffer
{
	public:
		IdentityBuffer(const ScalarType& type);

		vector<std::string> GetValues(const vector<std::string>& rowIds);		
		void Apply(const ColumnWrites& writes);
		RangeResult GetRows(const RangeRequest& range);
		Statistic GetStatistic();

	private:
		bool Include(void* rowId);
		bool Exclude(void* rowId);

		ScalarType _type;
		KeyTree* _rows;
		long long _count;
};

inline IdentityBuffer::IdentityBuffer(const ScalarType& type)
{
	_type = type;
	_rows = new KeyTree(_type);
	_count = 0;
}

inline Statistic IdentityBuffer::GetStatistic()
{
	Statistic stat;
	stat.total = _count;
	stat.unique = _count;
	return stat;
}

inline vector<std::string> IdentityBuffer::GetValues(const vector<std::string>& rowIds)
{
	vector<std::string> values(rowIds.size());
	for (unsigned int i = 0; i < rowIds.size(); i++)
	{
		auto path = _rows->GetPath(_type.GetPointer(rowIds[i]));
		if (path.Match)
			values[i] = rowIds[i];
	}

	return values;
}

inline void IdentityBuffer::Apply(const ColumnWrites& writes)
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

inline bool IdentityBuffer::Include(void* rowId)
{
	//TODO: Return Undo Information
	auto rowpath = _rows->GetPath(rowId);
	if (rowpath.Match)
		return false;
	else
	{		
		_rows->Insert(rowpath, rowId);
		_count++;
		return true;
	}
}

inline bool IdentityBuffer::Exclude(void* rowId)
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


inline RangeResult IdentityBuffer::GetRows(const RangeRequest& range)
{
	void* firstp = range.__isset.first ? _type.GetPointer(range.first.value) : NULL;
	void* lastp = range.__isset.last ? _type.GetPointer(range.last.value) : NULL;
	void* startId = range.__isset.rowID ? _type.GetPointer(range.rowID) : NULL;

	if (range.__isset.first && range.__isset.last)
	{		
		if (_type.Compare(firstp, lastp) > 0)
		{
			//Start is after end-> Invalid input.
			throw "Invalid range. Start is after end";
		}
	}

	//cache markers since we will use it several times
	KeyTree::iterator lastMarker = _rows->end();
	KeyTree::iterator firstMarker = _rows->begin();
	
	bool beginMatch = false;
	bool endMatch = false;

	KeyTree::iterator begin = 
		startId != NULL && range.ascending ? _rows->findNearest(startId, beginMatch) :
		range.__isset.first ? _rows->findNearest(firstp, beginMatch) :
		firstMarker;

	KeyTree::iterator end =  
		startId != NULL && !range.ascending ? _rows->findNearest(startId, endMatch) :
		range.__isset.last ? _rows->findNearest(lastp, endMatch) :
		lastMarker;

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

	ValueRowsList vrl;

	if (range.ascending)
	{		
		while (begin != end && !result.limited)
		{
			auto rowId = (void*)((*begin).key);

			if (!startFound && _type.Compare(rowId, startId) == 0)
			{				
				result.__set_bof(false);
				startFound = true;
				begin++;
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
			result.__set_limited(vrl.size() == range.limit);

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

			auto rowId = (void*)((*end).key);

			if (!startFound && _type.Compare(rowId, startId) == 0)
			{
				result.__set_eof(false);
				startFound = true;
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
			result.__set_limited(vrl.size() == range.limit);
		}

		//if we didn't make it through the entire set, reset the bof marker.
		if (result.limited && result.bof)
			result.__set_bof(false);
	}	

	result.__set_valueRowsList(vrl);

	return result;
}
