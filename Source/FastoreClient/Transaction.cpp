#include "Transaction.h"

using namespace fastore;

const boost::shared_ptr<Database> &Transaction::getDatabase() const
{
	return privateDatabase;
}

void Transaction::setDatabase(const boost::shared_ptr<Database> &value)
{
	privateDatabase = value;
}

const bool &Transaction::getReadIsolation() const
{
	return privateReadIsolation;
}

void Transaction::setReadIsolation(const bool &value)
{
	privateReadIsolation = value;
}

const bool &Transaction::getWriteIsolation() const
{
	return privateWriteIsolation;
}

void Transaction::setWriteIsolation(const bool &value)
{
	privateWriteIsolation = value;
}

Transaction::Transaction(const boost::shared_ptr<Database> &database, bool readIsolation, bool writeIsolation)
{
	setDatabase(database);
	setReadIsolation(readIsolation);
	setWriteIsolation(writeIsolation);
	// TODO: gen ID  - perhaps defer until needed; first read-write would obtain revision
	_transactionId = boost::shared_ptr<TransactionID>(new TransactionID());
	_transactionId->key = 0;
	_transactionId->revision = 0;

	_log = std::map<int, LogColumn*>();
}

Transaction::~Transaction()
{
	if (!_completed)
		Rollback();
}

void Transaction::Commit(bool flush = false)
{
	auto writes = GatherWrites();

	getDatabase()->Apply(writes, flush);

	_log.clear();
	_completed = true;
}

std::map<int, ColumnWrites*> Transaction::GatherWrites()
{
	std::map<int, ColumnWrites*> writesPerColumn = std::map<int, ColumnWrites*>();

	// Gather changes for each column
	for (std::map<int, LogColumn*>::const_iterator entry = _log.begin(); entry != _log.end(); ++entry)
	{
		boost::shared_ptr<ColumnWrites> writes = boost::shared_ptr<ColumnWrites>();

		// Process Includes
		for (auto include = entry->second->Includes.begin(); include != entry->second->Includes.end(); ++include)
		{
			if (writes == NULL)
			{
				writes = boost::shared_ptr<ColumnWrites>(new ColumnWrites());
				writes->includes = std::vector<Include>();
			}
			boost::shared_ptr<Alphora::Fastore::Include> inc = boost::make_shared<Fastore::Include>();
			inc->RowID = Fastore::Client::Encoder::Encode((*include)->Key);
			inc->Value = Fastore::Client::Encoder::Encode((*include)->Value);
			writes->Includes->Add(inc);
		}

		// Process Excludes
		for (unknown::const_iterator exclude = entry->Value->Excludes.begin(); exclude != entry->Value->Excludes.end(); ++exclude)
		{
			if (writes == nullptr)
				writes = boost::make_shared<ColumnWrites>();
			if (writes->Excludes == nullptr)
				writes->Excludes = std::vector<Fastore::Exclude*>();
			boost::shared_ptr<Alphora::Fastore::Exclude> ex = boost::make_shared<Fastore::Exclude> {RowID = Fastore::Client::Encoder::Encode(*exclude)};
			writes->Excludes->Add(ex);
		}

		if (writes != nullptr)
			writesPerColumn.insert(make_pair((*entry)->Key, writes));
	}

	return writesPerColumn;
}

void Transaction::Rollback()
{
	_log.clear();
	_completed = true;
}

boost::shared_ptr<RangeSet> Transaction::GetRange(int columnIds[], Range range, int limit, const boost::shared_ptr<object> &startId
{
	// Get the raw results
	auto raw = getDatabase()->GetRange(columnIds, range, limit, startId);

	// Find a per-column change map for each column in the selection
	auto changeMap = new LogColumn[sizeof(columnIds) / sizeof(columnIds[0])];
	auto anyMapped = false;
	for (int x = 0; x < sizeof(columnIds) / sizeof(columnIds[0]); x++)
	{
		boost::shared_ptr<LogColumn> col;
		if (_log.TryGetValue(columnIds[x], col))
		{
			anyMapped = true;
			changeMap[x] = col;
		}
		else
			changeMap[x].reset();
	}

	// Return raw if no changes to the requested columns
	if (!anyMapped)
		return raw;

	// Process excludes from results
	auto resultRows = std::vector<DataSet::DataSetRow>();
	for (Alphora::Fastore::Client::DataSet::const_iterator row = raw->getData()->begin(); row != raw->getData()->end(); ++row)
	{
		auto newRow = DataSet::DataSetRow {ID = (*row)->ID, Values = (*row)->Values};
		auto allNull = true;
		for (int i = 0; i < row->Values->Length; i++)
		{
			boost::shared_ptr<LogColumn> col = changeMap[i];
			if (col != nullptr)
			{
				if (std::find(col->Excludes.begin(), col->Excludes.end(), (*row)->Values[i]) != col->Excludes.end())
					newRow.Values[i].reset();
				else
				{
					allNull = false;
					newRow.Values[i] = (*row)->Values[i];
				}
			}
			else
				newRow.Values[i] = (*row)->Values[i];
		}
		if (!allNull)
			resultRows->Add(newRow);
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
	//    for (int i = 0; i < row.Values.Length; i++)
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
	auto result = boost::make_shared<DataSet>(resultRows->Count, sizeof(columnIds) / sizeof(columnIds[0]));
	for (var i = 0; i < result->getCount(); i++)
		result[i] = resultRows[i];
	raw->setData(result);

	return raw;
}

boost::shared_ptr<DataSet> Transaction::GetValues(int columnIds[], object rowIds[])
{
	// TODO: Filter/augment data for the transaction
	return getDatabase()->GetValues(columnIds, rowIds);
}

void Transaction::Include(int columnIds[], const boost::shared_ptr<object> &rowId, object row[])
{
	for (var i = 0; i < sizeof(columnIds) / sizeof(columnIds[0]); i++)
		EnsureColumnLog(columnIds[i])->Includes[rowId] = row[i];
}

void Transaction::Exclude(int columnIds[], const boost::shared_ptr<object> &rowId)
{
	for (var i = 0; i < sizeof(columnIds) / sizeof(columnIds[0]); i++)
		EnsureColumnLog(columnIds[i])->Excludes.insert(rowId);
}

Statistic *Transaction::GetStatistics(int columnIds[])
{
	return getDatabase()->GetStatistics(columnIds);
}

boost::shared_ptr<LogColumn> Transaction::EnsureColumnLog(int columnId)
{
	boost::shared_ptr<LogColumn> col;
	if (!_log.TryGetValue(columnId, col))
	{
		col = boost::make_shared<LogColumn>();
		_log.insert(make_pair(columnId, col));
	}
	return col;
}

std::map<int, TimeSpan> Transaction::Ping()
{
	return getDatabase()->Ping();
}
