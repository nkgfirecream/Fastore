#include "Table.h"
#include "..\FastoreClient\Dictionary.h"
#include "..\FastoreClient\Encoder.h"
#include <boost\assign\list_of.hpp>

using namespace fastore::module;
using namespace fastore::client;
using namespace boost::assign;

Table::Table(provider::Connection* connection, std::string name, std::vector<client::ColumnDef> columns) : _connection(connection), _name(name), _columns(columns) { }

void Table::begin()
{
	_transaction = _connection->_database->Begin(true, true);
}

void Table::sync()
{
	//Do nothing for now.. we may need to expose two stage transactions on the client.
}

void Table::commit()
{
	_transaction->Commit();
	_transaction.reset();
}

void Table::rollback()
{
	_transaction->Rollback();
	_transaction.reset();
}

void Table::create()
{
	 // Pull a list of candidate pods
    Range podQuery;
    podQuery.ColumnID = Dictionary::PodColumnPodID;
    podQuery.Ascending = true;
	//TODO: Gather pods - we may have more than 2000
    auto podIds = _connection->_database->GetRange(Dictionary::PodColumnColumns, podQuery, 2000);
    if (podIds.Data.size() == 0)
        throw "FastoreModule can't create a new table. The hive has no pods. The hive must be initialized first.";


	//TODO: Determine key -- if it's more than one column, we need to create a surrogate.
	//For now, just assume the first column is the key.


	//var minKey = TableVar.Keys.MinimumKey(false, false);
	//if (minKey != null && minKey.Columns.Count != 1)
	//	minKey = null;
	//var rowIDColumn = minKey != null && RowIDCandidateType(minKey.Columns[0].DataType) ? minKey.Columns[0] : null;
	//var rowIDmappedType = rowIDColumn == null ? "Int" : MapTypeNames(rowIDColumn.DataType);

    // Start distributing columns on a random pod. Otherwise, we will always start on the first pod
    int nextPod = rand() % (podIds.Data.size() - 1);

    //This is so we have quick access to all the ids (for queries). Otherwise, we have to iterate the 
    //TableVar Columns and pull the id each time.
    //List<int> columnIds = new List<int>();
	for (int i = 0; i < _columns.size(); i++)
	{
		auto column = _columns[i];
		int columnId = 0;
		auto combinedName = _name + "." + column.Name;

		// Attempt to find the column by name
		Range query;
		query.ColumnID = Dictionary::ColumnName;

		client::RangeBound bound;
		bound.Bound = combinedName;
		bound.Inclusive = true;
		query.Start = bound;
		query.End = bound;
		//Just pull one row. If any exist we have a problem.
		auto result = _connection->_database->GetRange(ColumnIDs(), query, 1);

		if (result.Data.size() > 0)
		{
			throw "Column " + combinedName + " already exists in store. Cannot create column";
		}
		else
		{
			columnId = (int)_connection->_generator->Generate(Dictionary::ColumnID);			
			_columnIds.push_back(columnId);
			_columns[i].ColumnID = columnId;
			createColumn(column, combinedName, _columns[0], podIds, nextPod);
		}
	}
}

void Table::connect()
{
	//Try to locate all the columns (range over the columns table with "tablename.") and gather their ids
	//so that we can use them.
	//Try to locate rowId column if the column exists. 
}

void Table::drop()
{
	for (auto col : _columnIds)
	{
        // Pull a list of the current repos so we can drop them all.
        Range repoQuery;
        repoQuery.ColumnID = Dictionary::PodColumnColumnID;
        repoQuery.Ascending = true;
		client::RangeBound bound;
		bound.Inclusive = true;
		bound.Bound = client::Encoder<communication::ColumnID>::Encode(col);
        repoQuery.Start = bound;
        repoQuery.End = bound;

		auto repoIds = _connection->_database->GetRange(list_of<communication::ColumnID>(Dictionary::PodColumnColumnID), repoQuery, 2000);

        for (int i = 0; i < repoIds.Data.size(); i++)
        {
            _connection->_database->Exclude(Dictionary::PodColumnColumns, repoIds.Data[i].ID);
        }

		Range query;
        query.ColumnID = Dictionary::ColumnID;
        query.Start = bound;
        query.End = bound;

        auto columnExists = _connection->_database->GetRange(list_of<communication::ColumnID>(Dictionary::ColumnID), query, 2000);

        if (columnExists.Data.size() > 0)
        {
            _connection->_database->Exclude(Dictionary::ColumnColumns,  client::Encoder<communication::ColumnID>::Encode(col));
        }
	}
}

void Table::disconnect()
{
	//Do nothing for now...
}

void Table::createColumn(client::ColumnDef& column, std::string combinedName, client::ColumnDef& rowIDColumn, RangeSet& podIds, int nextPod)
{
	//TODO: Determine the storage pod - default, but let the user override -- we'll need to extend the sql to support this.
	auto podId = podIds.Data[nextPod++ % podIds.Data.size()].Values[0];


	//TODO: Make workers smart enough to create/instantiate a column within one transaction.
	//(They currently don't check for actions to perform until the end of the transaction, which means they may miss part of it currently)
	auto transaction = _connection->_database->Begin(true, true);
	transaction->Include
	(
		Dictionary::ColumnColumns,
		client::Encoder<communication::ColumnID>::Encode(column.ColumnID),
		list_of<std::string>
		(client::Encoder<communication::ColumnID>::Encode(column.ColumnID))
		(combinedName)
		(column.Type)
		(column.IDType)
		(client::Encoder<client::BufferType>::Encode(column.BufferType))
		(client::Encoder<bool>::Encode(column.Required))
	);
	transaction->Commit();

	transaction = _connection->_database->Begin(true, true);
	transaction->Include
	(
		Dictionary::PodColumnColumns,
		client::Encoder<long long>::Encode(_connection->_generator->Generate(Dictionary::PodColumnPodID)),
		list_of<std::string>
		(podId)
		(client::Encoder<communication::ColumnID>::Encode(column.ColumnID))
	);
	transaction->Commit();		
}

void Table::bestIndex(sqlite3_index_info* info)
{
	//Inputs..

	//Where clause constraints
	for (int i = 0; i < info->nConstraint; ++i)
	{
		auto pConstraint = &info->aConstraint[i];
		pConstraint->iColumn; // Which column
		pConstraint->op; // operator
		pConstraint->usable; // true if this constraint is usable
	}

	//Orderby clause (number of orders in order by)
	for (int i = 0; i < info->nOrderBy; ++i)
	{
		auto pOrder = &info->aOrderBy[i];
		pOrder->desc; // descending
		pOrder->iColumn; // Which column
	}

	//Outputs...
	info->aConstraintUsage->argvIndex; // if >0, constraint is part of argv to xFilter
	info->aConstraintUsage->omit; // String, possibly obtained from sqlite3_malloc
	info->idxNum; // Number used to identify the index
	info->idxStr; // String, possibly obtained from sqlite3_malloc
	info->needToFreeIdxStr; //  Free idxStr using sqlite3_free() if true
	info->orderByConsumed; // True if output is already ordered
	info->estimatedCost; //  Estimated cost of using this index
}

client::RangeSet Table::getRange(client::Range& range, boost::optional<std::string>& startId)
{
	if (_transaction != NULL)
		return _transaction->GetRange(_columnIds, range, 500, startId);
	else
		return _connection->_database->GetRange(_columnIds, range, 500, startId);
}

void Table::deleteRow(sqlite3_value* rowId)
{
	int id = sqlite3_value_int(rowId);
	std::string rowid = Encoder<int>::Encode(id);

	if (_transaction != NULL)
		_transaction->Exclude(_columnIds, rowid);
	else
		_connection->_database->Exclude(_columnIds, rowid);
}

void Table::insertRow(int argc, sqlite3_value **argv, sqlite3_int64 *pRowid)
{
	if (argc != _columnIds.size() + 2)
		throw "Wrong number of values for row";

	sqlite3_int64 rowIdInt;
	if (sqlite3_value_type(argv[1]) != SQLITE_NULL)
	{
		rowIdInt = sqlite3_value_int64(argv[1]);
	}
	else
	{
		//TODO: generate on key column..
		rowIdInt = _connection->_generator->Generate(_columnIds[0]);
		*pRowid = rowIdInt;
	}

	std::string rowid = Encoder<int>::Encode(rowIdInt);

	std::vector<std::string> row;

	for (int i = 0; i < _columns.size(); ++i)
	{
		auto pValue = argv[i+2];
		if (sqlite3_value_type(pValue) != SQLITE_NULL)
		{
			std::string type = _columns[i].Type;

			std::string value;
			if (type == "Bool")
				value = Encoder<bool>::Encode((bool)sqlite3_value_int(pValue));
			else if (type == "Int")
				value = Encoder<int>::Encode(sqlite3_value_int(pValue));
			else if (type == "Long")
				value = Encoder<long long>::Encode(sqlite3_value_int64(pValue));
			else if (type == "String")
				value = std::string((const char *)sqlite3_value_text(pValue));
			else if (type == "WString")

			row.push_back(value);
		}
		else if (!_columns[i].Required)
		{
			//TODO: NULL marker;
			row.push_back("");
		}
		else
			throw "Got a NULL value on a required row!";
	}	

	if (_transaction != NULL)
		_transaction->Include(_columnIds, rowid, row);
	else
		_connection->_database->Include(_columnIds, rowid, row);
}