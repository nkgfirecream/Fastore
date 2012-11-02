#pragma once

#include <boost\bimap.hpp>
#include <boost\bimap\multiset_of.hpp>
#include <boost/bimap/support/lambda.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost\optional.hpp>
#include "IColumnBuffer.h"
#include "BimapConversions.h"

template<typename T>
class MultiBimapBuffer : public IColumnBuffer
{
	typedef boost::bimap<boost::bimaps::multiset_of<T>, boost::bimaps::unordered_set_of<int64_t>> bm_type;

	public:
		MultiBimapBuffer();

		std::vector<OptionalValue> GetValues(const std::vector<std::string>& rowIds);		
		void Apply(const ColumnWrites& writes);
		RangeResult GetRows(const RangeRequest& range);
		Statistic GetStatistic();

	private:
		bool Include(int64_t rowId, T& value);
		bool Exclude(int64_t rowId);
		boost::optional<T> GetValue(int64_t rowId);	

		bm_type _data;
};

template<typename T>
MultiBimapBuffer<T>::MultiBimapBuffer() { }

template<typename T>
Statistic MultiBimapBuffer<T>::GetStatistic()
{
	Statistic stat;
	stat.total = _data.right.size();
	stat.unique = _data.left.size();
	return stat;
}

template<typename T>
std::vector<OptionalValue> MultiBimapBuffer<T>::GetValues(const std::vector<std::string>& rowIds)
{
	vector<OptionalValue> values(rowIds.size());
	for (unsigned int i = 0; i < rowIds.size(); ++i)
	{
		boost::optional<T> result = GetValue(GetValueFromString<int64_t>(rowIds[i]));
		if (result)
		{
			std::string value = GetStringFromValue<T>(*result);
			values[i].__set_value(value);
		}
	}

	return values;
}

template<typename T>
boost::optional<T> MultiBimapBuffer<T>::GetValue(int64_t rowId)
{
	auto result = _data.right.find(rowId);	
	if (result != _data.right.end())
		return boost::optional<T>(result->second);
	else
		return boost::optional<T>();
}

template<typename T>
void MultiBimapBuffer<T>::Apply(const ColumnWrites& writes)
{
	for (auto exstart = writes.excludes.begin(), exend = writes.excludes.end(); exstart != exend; ++exstart)
	{
		int64_t rowId = GetValueFromString<int64_t>((*exstart).rowID);
		Exclude(rowId);
	}

	for (auto incstart =writes.includes.begin(), incend = writes.includes.end(); incstart != incend; ++incstart)
	{
		T value = GetValueFromString<T>((*incstart).value);
		int64_t rowId =  GetValueFromString<int64_t>((*incstart).rowID);
		Include(rowId, value);
	}
}

template<typename T>
bool MultiBimapBuffer<T>::Include(int64_t rowId, T& value)
{
	auto result = _data.insert(bm_type::value_type(value, rowId));
	return result.second;
}

template<typename T>
bool MultiBimapBuffer<T>::Exclude(int64_t rowId)
{
	auto result = _data.right.erase(rowId);
	return result > 0;
}

template<typename T>
RangeResult MultiBimapBuffer<T>::GetRows(const RangeRequest& range)
{
	boost::optional<T> firstp = range.__isset.first ? GetValueFromString<T>(range.first.value) : boost::optional<T>();
	boost::optional<T> lastp = range.__isset.last ?  GetValueFromString<T>(range.last.value) : boost::optional<T>();
	boost::optional<int64_t> startId = range.__isset.rowID ?  GetValueFromString<int64_t>(range.rowID) : boost::optional<int64_t>();
	
	RangeResult result;	
	ValueRowsList vrl;

	if (firstp && lastp &&  ((range.ascending &&  *firstp > *lastp) || (!range.ascending && *firstp < *lastp)))
	{		
		//Disjoint constraints - (e.g. < 2 && > 5). Return empty set.
		result.__set_valueRowsList(vrl);
		result.__set_bof(true);
		result.__set_eof(true);
		result.__set_limited(false);
	}

	bm_type::left_iterator firstMarker = range.ascending ? _data.left.begin() : _data.left.end();
	bm_type::left_iterator lastMarker = range.ascending ? _data.left.end() : _data.left.begin();
	

	bool bInclusive = range.__isset.first ? range.first.inclusive : true;
	bool eInclusive = range.__isset.last ? range.last.inclusive : true;
		
	bm_type::left_iterator begin = 
	startId ? _data.project_left(_data.right.find(*startId)) :
	firstp ?  
		(range.ascending ? 
			(bInclusive? 
			_data.left.lower_bound(*firstp) : 
			_data.left.upper_bound(*firstp)) 
			:
			(bInclusive? 
			_data.left.upper_bound(*firstp) : 
			_data.left.lower_bound(*firstp)))
			:
	firstMarker;

	bm_type::left_iterator end = 
	lastp ?  
		(!range.ascending ? 
			(eInclusive? 
			_data.left.lower_bound(*lastp) : 
			_data.left.upper_bound(*lastp)) 
			:
			(eInclusive? 
			_data.left.upper_bound(*lastp) : 
			_data.left.lower_bound(*lastp)))
			:
	lastMarker;

	result.__set_bof(begin == firstMarker);
	result.__set_eof(end == lastMarker);

	bool startFound = !startId;

	int num = 0;

	if (range.ascending)
	{
		while (begin != end && !result.limited)
		{
			if (!startFound)
			{
				if (begin->second == *startId)
				{
					startFound = true;
					result.__set_bof(false);
					++begin;
					continue;
				}
				else
				{
					throw "Start id not found in given value";
				}					
			}
			
			T key = begin->first;
			std::vector<std::string> rowIds;
			while (begin != end && begin->first == key && !result.limited)
			{
				int64_t rowId = begin->second;
				rowIds.push_back(GetStringFromValue<int64_t>(rowId));
				++num;
				++begin;

				result.__set_limited(num == range.limit);
			}

			if (rowIds.size() > 0)
			{
				ValueRows vr;
				
				vr.__set_value(GetStringFromValue<T>(key));
				vr.__set_rowIDs(rowIds);
				vrl.push_back(vr);
			}
		}

		//if we didn't make it through the entire set, reset the eof marker.
		if (result.limited && result.eof)
			result.__set_eof(false);

		result.__set_valueRowsList(vrl);
		return result;
	}
	else
	{
		auto rbegin = reverse_iterator<bm_type::left_iterator>(begin);
		auto rend = reverse_iterator<bm_type::left_iterator>(end);
		while (rbegin != rend && !result.limited)
		{
			if (!startFound)
			{
				--rbegin;
				if (rbegin->second == *startId)
				{
					startFound = true;
					result.__set_bof(false);	
					++rbegin;
					continue;
				}
				else
				{
					throw "Start id not found in given value";
				}					
			}
			
			T key = rbegin->first;
			std::vector<std::string> rowIds;
			while (rbegin != rend && rbegin->first == key && !result.limited)
			{
				int64_t rowId = rbegin->second;
				rowIds.push_back(GetStringFromValue<int64_t>(rowId));
				++num;
				++rbegin;

				result.__set_limited(num == range.limit);
			}

			if (rowIds.size() > 0)
			{
				ValueRows vr;
				
				vr.__set_value(GetStringFromValue<T>(key));
				vr.__set_rowIDs(rowIds);
				vrl.push_back(vr);
			}
		}

		//if we didn't make it through the entire set, reset the eof marker.
		if (result.limited && result.eof)
			result.__set_eof(false);

		result.__set_valueRowsList(vrl);
		return result;
	}
}
