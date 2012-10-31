#include "Transaction.h"

using namespace fastore::client;

const Database &Transaction::getDatabase() const
{
	return privateDatabase;
}

const bool &Transaction::getReadIsolation() const
{
	return privateReadIsolation;
}

const bool &Transaction::getWriteIsolation() const
{
	return privateWriteIsolation;
}

Transaction::Transaction(Database& database, bool readIsolation, bool writeIsolation)
	: privateDatabase(database), privateReadIsolation(readIsolation), privateWriteIsolation(writeIsolation)
{
	_transactionId.__set_key(0);
	_transactionId.__set_revision(0);
	_log = std::map<ColumnID, LogColumn>();
}

Transaction::~Transaction()
{
	if (!_completed)
		Rollback();
}

void Transaction::Commit(bool flush)
{
	std::map<ColumnID,  ColumnWrites> writes;
	GatherWrites(writes);

	privateDatabase.Apply(writes, flush);

	_log.clear();
	_completed = true;
}

void Transaction::GatherWrites(std::map<ColumnID,  ColumnWrites>& output)
{
	// Gather changes for each column
	for (auto entry = _log.begin(); entry != _log.end(); ++entry)
	{
		if(entry->second.Includes.size() == 0 && entry->second.Excludes.size() == 0)
		{
			continue;
		}

		output.insert(std::make_pair(entry->first, ColumnWrites()));
		ColumnWrites& writes = output[entry->first];

		// Process Includes
		if (entry->second.Includes.size() > 0)
		{
			writes.__set_includes(std::vector<fastore::communication::Include>(entry->second.Includes.size()));
			int i = 0;
			for (auto include = entry->second.Includes.begin(); include != entry->second.Includes.end(); ++include, ++i)
			{
				writes.includes[i].__set_rowID(include->first);
				writes.includes[i].__set_value(include->second);
			}
		}

		// Process Excludes
		if (entry->second.Excludes.size() > 0)
		{
			writes.__set_excludes(std::vector<fastore::communication::Exclude>(entry->second.Excludes.size()));
			int i = 0;
			for (auto exclude = entry->second.Excludes.begin(); exclude != entry->second.Excludes.end(); ++exclude, ++i)
			{
				writes.excludes[i].__set_rowID(*exclude);
			}
		}			
	}
}

void Transaction::Rollback()
{
	_log.clear();
	_completed = true;
}

RangeSet Transaction::GetRange(const ColumnIDs& columnIds, const Range& range, const int limit, const boost::optional<std::string> &startId)
{
	// Get the raw results
	auto raw = privateDatabase.GetRange(columnIds, range, limit, startId);

	// Find a per-column change map for each column in the selection
	std::vector<LogColumn> changeMap(columnIds.size());
	auto anyMapped = false;
	for (size_t x = 0; x < columnIds.size(); ++x)
	{
		auto col = _log.find(columnIds[x]);
		if (col != _log.end())
		{
			anyMapped = true;
			changeMap[x] = col->second;
		}
	}

	// Return raw if no changes to the requested columns
	if (!anyMapped)
		return raw;

	// Process excludes from results
	std::vector<DataSetRow> resultRows; 
	for (auto row = raw.Data.begin(); row != raw.Data.end(); ++row)
	{
		DataSetRow newRow(row->Values.size());
		newRow.ID = row->ID;
		newRow.Values = row->Values;
		auto allNull = true;

		for (size_t i = 0; i < row->Values.size(); i++)
		{
			LogColumn col = changeMap[i];
			if (!col.Excludes.empty() || !col.Includes.empty())
			{
				if (std::find(col.Excludes.begin(), col.Excludes.end(), row->ID) != col.Excludes.end())
				{
					//Set null marker
					newRow.Values[i] = fastore::communication::OptionalValue();
				}
				else
				{
					allNull = false;
					newRow.Values[i] = row->Values[i];
				}
			}
			else
				newRow.Values[i] = row->Values[i];
		}

		if (!allNull)
			resultRows.push_back(newRow);
	}

	// TODO: handle includes - probably need to keep a shadow of column buffers to do the merging with
	// Cases for a given range
	// 1 - Local exclusions within range
	// 2 - Local inclusions within range
	// 3 - Local updates to a row that:
	//      A - Move a row into the range
	//      B - Move a row out of the range

	//Update case: a change to a value in a row
	//This could cause the rows to get out of order if you've updated a row in the range.
	//foreach (var row in resultRows)
	//{
	//    for (size_t i = 0; i < row.Values.Length; i++)
	//    {
	//        LogColumn col = changeMap[i];
	//        if (col != null)
	//        {
	//            if (col.Includes.ContainsKey(row.ID))
	//            {
	//                row.Values[i] = col.Includes[row.ID];
	//            }
	//        }
	//    }
	//}

	//Insert case: Include new rows that are not present in our get range.

	// Turn the rows back into a dataset
	DataSet result(resultRows.size(), columnIds.size());
	for (size_t i = 0; i < result.size(); i++)
		result[i] = resultRows[i];

	raw.Data = result;

	return raw;
}

DataSet Transaction::GetValues(const ColumnIDs& columnIds, const std::vector<std::string>& rowIds)
{
	// TODO: Filter/augment data for the transaction
	return privateDatabase.GetValues(columnIds, rowIds);
}

void Transaction::Include(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row)
{
  for (size_t i = 0; i < columnIds.size(); ++i)
	{
		LogColumn& column = EnsureColumnLog(columnIds[i]);
		column.Includes[rowId] = row[i];
	}
}

void Transaction::Exclude(const ColumnIDs& columnIds, const std::string& rowId)
{
  for (size_t i = 0; i < columnIds.size(); ++i)
	{
		LogColumn& column = EnsureColumnLog(columnIds[i]);
		column.Excludes.insert(rowId);
	}
}

std::vector<Statistic> Transaction::GetStatistics(const ColumnIDs& columnIds)
{
	return privateDatabase.GetStatistics(columnIds);
}

Transaction::LogColumn& Transaction::EnsureColumnLog(const ColumnID& columnId)
{
	auto iter = _log.find(columnId);
	if (iter == _log.end())
	{
		_log.insert(std::pair<ColumnID, LogColumn>(columnId, LogColumn()));
	}
	
	return _log[columnId];	
}

std::map<HostID, long long> Transaction::Ping()
{
	return privateDatabase.Ping();
}
