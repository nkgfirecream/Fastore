#include "Transaction.h"
#include <algorithm>
#include <boost/format.hpp>

using namespace fastore::client;
using namespace fastore::communication;

const Database &Transaction::getDatabase() const
{
	return _database;
}

Transaction::Transaction(Database& database, bool readsConflict)
	: _database(database), _readsConflict(readsConflict)
{
	_transactionId = database.generateTransactionID();
	_log = std::unordered_map<ColumnID, LogColumn>();
}

Transaction::~Transaction()
{
	if (!_completed)
		rollback();
}

void Transaction::commit(bool flush)
{
	std::map<ColumnID,  ColumnWrites> writes;
	gatherWrites(writes);
	_database.apply(_transactionId, writes, flush);

	_log.clear();
	_completed = true;
}

void Transaction::gatherWrites(std::map<ColumnID, ColumnWrites>& output)
{
	// Gather changes for each column
	for (auto entry = _log.begin(); entry != _log.end(); ++entry)
	{
		auto includeTotal = entry->second.includes->GetStatistic().total;
		// Skip any empty columns
		if (includeTotal == 0 && entry->second.excludes.size() == 0 && entry->second.reads.size() == 0)
			continue;

		output.insert(std::make_pair(entry->first, ColumnWrites()));
		ColumnWrites& writes = output[entry->first];

		// Process Includes
		if (includeTotal > 0)
		{
			writes.__set_includes(std::vector<fastore::communication::Cell>(includeTotal));
			RangeRequest range;
			range.limit = std::numeric_limits<decltype(RangeRequest::limit)>::max();
			auto rows = entry->second.includes->GetRows(range);
			int i = 0;
			for (auto value = rows.valueRowsList.begin(); value != rows.valueRowsList.end(); ++value)
				for (auto row = value->rowIDs.begin(); row != value->rowIDs.end(); ++row)
				{
					writes.includes[i].__set_rowID(*row);
					writes.includes[i].__set_value(value->value);
					++i;
				}
		}

		// Process Excludes
		if (entry->second.excludes.size() > 0)
		{
			writes.__set_excludes(std::vector<fastore::communication::Cell>(entry->second.excludes.size()));
			int i = 0;
			for (auto exclude = entry->second.excludes.begin(); exclude != entry->second.excludes.end(); ++exclude, ++i)
			{
				writes.excludes[i].__set_rowID(*exclude);
			}
		}			
	}
}

void Transaction::rollback()
{
	_log.clear();
	_completed = true;
}

DataSetRow addRow(std::vector<Transaction::LogColumn*> logMap, RangeResult localRange, size_t rangeColumnID, size_t &valueIndex, size_t &rowIndex)
{
	DataSetRow newRow(logMap.size());
	newRow.ID = localRange.valueRowsList[valueIndex].rowIDs[rowIndex];

	// Populate the row's values
	for (size_t i = 0; i < logMap.size(); i++)
	{
		if (i == rangeColumnID)
			newRow.Values[i].__set_value(localRange.valueRowsList[valueIndex].rowIDs[rowIndex]);
		else
		{
			auto colLog = logMap[i];		
			if (colLog != nullptr)
				newRow.Values[i] = colLog->includes->GetValue(newRow.ID);
		}
	}

	// Advance to the next value/row
	++rowIndex;
	if (rowIndex >= localRange.valueRowsList[valueIndex].rowIDs.size())
	{
		++valueIndex;
		rowIndex = 0;
	}

	return newRow;
}

std::vector<Transaction::LogColumn*> Transaction::buildLogMap(const ColumnIDs& columnIds, bool &anyMapped)
{
	std::vector<Transaction::LogColumn*> result(columnIds.size());
	anyMapped = false;
	for (size_t i = 0; i < columnIds.size(); ++i)
	{
		auto col = _log.find(columnIds[i]);
		if (col != _log.end())
		{
			anyMapped = true;
			result[i] = &col->second;
		}
	}
	return result;
}

void Transaction::applyColumnOverrides(LogColumn &colLog, DataSetRow &row, size_t colIndex)
{
	// Look for an exclude that might clear the value
	auto excluded = colLog.excludes.find(row.ID);
	if (excluded != colLog.excludes.end())
	{
		row.Values[colIndex].value = std::string();
		row.Values[colIndex].__isset.value = false;
	}

	// Look for an include that might set/override the value
	auto included = colLog.includes->GetValue(row.ID);
	if (included.__isset.value)
		row.Values[colIndex] = included;
}

/*
	getRange implementation notes:
		* Essentially a merge join of the local and remote results of GetRange.
		* An inclusion or exclusion of a row ID of the range column ID represents existence of a row in the result
		* Inclusions and exclusions involving other columns represent clearing or setting of fields in the row
		* Merge join depends on local and remote GetRanges returning the values and within values, rowIDs in 
		  sorted order.  (If row IDs aren't also sorted, this logic will be wrong)
		* The remote rows are used and changed as needed to avoid a copy
*/
RangeSet Transaction::getRange(const ColumnIDs& columnIds, const Range& range, const int limit, const boost::optional<std::string> &startId)
{
	// Get the remote results
	auto remote = _database.getRange(columnIds, range, limit, startId);

	// Build a log entry per column for quick access
	bool anyMapped;
	auto logMap = buildLogMap(columnIds, anyMapped);

	// Return remote if no overlap between log and selection
	if (!anyMapped)
		return remote;

	RangeSet result;
	result.Bof = remote.Bof;
	result.Eof = remote.Eof;
	result.Limited = remote.Limited;

	auto rangeLog = logMap[range.ColumnID];		

	// Attempt to find any local includes for the range column
	RangeResult localRange;
	if (rangeLog != nullptr)
	{
		auto colRange = rangeToRangeRequest(range, limit);
		localRange = rangeLog->includes->GetRows(colRange);

		// Only EOF or BOF if both local and remote agree
		result.Bof &= localRange.bof;
		result.Eof &= localRange.eof;
	}

	// Variables to advance through local range during merge with remote
	size_t valueIndex = 0;
	size_t rowIndex = 0;

	// Merge each row of remote with local rows to build result
	for (auto remoteRow = remote.Data.begin(); remoteRow != remote.Data.end(); ++remoteRow)
	{	
		// While there is a locally included row before the present one, insert it							
		int valueComp, rowComp;
		while 
		(
			rangeLog != nullptr
				&& valueIndex < localRange.valueRowsList.size() 
				&& 
				(
					// Value precedes the selected one
					(valueComp = rangeLog->valueType.Compare(localRange.valueRowsList[valueIndex].value.data(), (*remoteRow)[range.ColumnID].value.data())) < 0
						// Or value equals selected on and the row ID precedes the selected one
						|| (valueComp == 0 && (rowComp = rangeLog->rowType.Compare(localRange.valueRowsList[valueIndex].rowIDs[rowIndex].data(), remoteRow->ID.data())) <= 0)
				)
		)
		{
			if (valueComp < 0 || rowComp < 0)
			{
				DataSetRow remoteRow = addRow(logMap, localRange, range.ColumnID, valueIndex, rowIndex);
				result.Data.push_back(remoteRow);
			}
		}

		// Add rows where there is no local range or the ranged column isn't excluded
		if (rangeLog == nullptr || rangeLog->excludes.find(remoteRow->ID) != rangeLog->excludes.end())
		{
			// Process includes and excludes for the remote row
			for (size_t i = 0; i < remoteRow->Values.size(); i++)
			{
				auto colLog = logMap[i];		
				if (colLog != nullptr && i != range.ColumnID)
					applyColumnOverrides(*colLog, *remoteRow, i);
			}

			result.Data.push_back(*remoteRow);
		}
	}

	// Append any remaining local includes
	while 
	(
		rangeLog != nullptr
			&& valueIndex < localRange.valueRowsList.size() 
	)
	{
		DataSetRow row = addRow(logMap, localRange, range.ColumnID, valueIndex, rowIndex);
		result.Data.push_back(row);
	}

	return result;
}

DataSet Transaction::getValues(const ColumnIDs& columnIds, const std::vector<std::string>& rowIds)
{
	// Get the remote values
	auto remote = _database.getValues(columnIds, rowIds);

	// Build a log entry per column for quick access
	bool anyMapped;
	auto logMap = buildLogMap(columnIds, anyMapped);

	// Return remote if no overlap between log and selection
	if (!anyMapped)
		return remote;

	// Process each row of remote against local changes
	for (auto remoteRow = remote.begin(); remoteRow != remote.end(); ++remoteRow)
	{	
		// Process includes and excludes for the remote row
		for (size_t i = 0; i < remoteRow->Values.size(); i++)
		{
			auto colLog = logMap[i];		
			if (colLog != nullptr)
				applyColumnOverrides(*colLog, *remoteRow, i);
		}
	}

	return remote;
}

void Transaction::include(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row)
{
	for (size_t i = 0; i < columnIds.size(); ++i)
	{
		// TODO: Introduce non-batch calls into buffers so that this is easier and faster
		LogColumn& column = ensureColumnLog(columnIds[i]);
		RangeRequest range;
		range.first.value = row[i];
		range.first.inclusive = true;
		range.rowID = rowId;
		range.last = range.first;
		if (column.includes->GetRows(range).valueRowsList.size() == 0)
		{
			ColumnWrites writes;
			fastore::communication::Cell include;
			include.__set_rowID(rowId);
			include.__set_value(row[i]);
			writes.includes.push_back(include);
			column.includes->Apply(writes);
		}
	}
}

void Transaction::exclude(const ColumnIDs& columnIds, const std::string& rowId)
{
	for (size_t i = 0; i < columnIds.size(); ++i)
	{
		LogColumn& column = ensureColumnLog(columnIds[i]);
		column.excludes.emplace(rowId);
	}
}

/*
	Note: unique statistics within a transaction are only approximate, and total assumes that 
	all excluded columns are present in the remote.
*/
std::vector<Statistic> Transaction::getStatistics(const ColumnIDs& columnIds)
{
	auto remote = _database.getStatistics(columnIds);

	// Build a log entry per column for quick access
	bool anyMapped;
	auto logMap = buildLogMap(columnIds, anyMapped);

	// Return remote if no overlap between log and selection
	if (!anyMapped)
		return remote;

	// Adjust approximately based on local changes
	for (size_t i = 0; i < logMap.size(); i++)
	{
		auto colLog = logMap[i];		
		if (colLog != nullptr)
		{
			auto localStat = colLog->includes->GetStatistic();
			remote[i].total = std::max<int64_t>(0, remote[i].total + localStat.total - colLog->excludes.size());
			// TODO: Better estimate of unique based on ratio
			remote[i].unique = std::max<int64_t>(0, remote[i].unique + localStat.unique - colLog->excludes.size());
		}
	}

	return remote;
}

Transaction::LogColumn& Transaction::ensureColumnLog(const ColumnID& columnId)
{
	auto iter = _log.find(columnId);
	if (iter == _log.end())
	{
		auto schema = _database.getSchema();
		auto def = schema.find(columnId);
		if (def != schema.end())
		{
			auto inserted = _log.insert(std::make_pair(columnId, LogColumn(def->second.RowIDType, def->second.ValueType)));
			return inserted.first->second;
		}
		else
			throw ClientException((boost::format("Unable to locate column (%i) in the schema.") % columnId).str());
	}
	else
		return iter->second;
}
