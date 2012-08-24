#include <sqlite3.h>
#include "Table.h"
#include "..\FastoreClient\Dictionary.h"
#include "..\FastoreClient\Encoder.h"
#include "..\FastoreClient\Transaction.h"
#include <boost\assign\list_of.hpp>

using namespace fastore::module;
using namespace fastore::client;
using namespace boost::assign;

Table::Table(Connection* connection, std::string& name, std::vector<client::ColumnDef>& columns) : _connection(connection), _name(name), _columns(columns) { }

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
    podQuery.ColumnID = Dictionary::PodID;
    podQuery.Ascending = true;

	std::vector<ColumnID> podidv;
	podidv.push_back(Dictionary::PodID);

	//TODO: Gather pods - we may have more than 2000
    auto podIds = _connection->_database->GetRange(podidv, podQuery, 2000);
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
		auto combinedName = _name + "." + _columns[i].Name;

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
			int columnId = (int)_connection->_generator->Generate(Dictionary::ColumnID);			
			_columnIds.push_back(columnId);
			_columns[i].ColumnID = columnId;
			createColumn(_columns[i], combinedName, _columns[0], podIds, nextPod);
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

void Table::createColumn(client::ColumnDef& column, std::string& combinedName, client::ColumnDef& rowIDColumn, RangeSet& podIds, int nextPod)
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
	////Inputs..
	//double constraintCost = 0.0;
	//int constraintColumn = -1;
	//bool constraintSupported = true;
	//bool hasConstraint;
	////Constraints Fastore can support...
	////On a single column only
	//// 1 =
	//// 1 </<=
	//// 1 >/>=
	//// 1 </<= + 1 >/>=

	////Can't support Match
	//
	////We can't support more than two constraints, ever.
	//if (info->nConstraint > 2)
	//	//Fail -- whatever that means
	//	constraintSupported = false;
	////If we have two constraints, they must be on the same column.
	//if (info->nConstraint == 2 && (info->aConstraint[0].iColumn != info->aConstraint[1].iColumn))
	//	//Fail -- whatever that means.
	//	constraintSupported = false;

	////We don't support match at all.
	//for (int i = 0; i < info->nConstraint; ++i)
	//{
	//	if (info->aConstraint[i].op == SQLITE_INDEX_CONSTRAINT_MATCH)
	//		constraintSupported = false;
	//}

	////All other combinations should work, right? (assuming valid comibinations from SQlite -- < 5 && < 10 doesn't make too much sense... right? the docs says it will pass in an arbitrary EXPR. If that expression is more complicated than a value...)
	//if (constraintSupported == true && info->nConstraint > 0)
	//{
	//	hasConstraint = true;
	//	constraintColumn == info->aConstraint[0].iColumn;
	//}

	////Estimate constraint cost..
	//if (hasConstraint && constraintSupported)
	//{
	//	if (info->aConstraint[0].op == SQLITE_INDEX_CONSTRAINT_EQ && info->nConstraint == 1)
	//		constraintCost += 20.0; //Equality constraints should produce the fewest numbers of rows, but still involves a search.
	//	else if (info->nConstraint == 2)
	//		constraintCost += 50.0; //Two constaints will produce a middle number of rows..
	//	else if (info->nConstraint == 1)
	//		constraintCost += 80.0; //One constraint will produce more rows, and a single search.
	//
	//	if (_columns[constraintColumn].Type == "String" || _columns[constraintColumn].Type == "WString")
	//		constraintCost += 10.0; // Searching over strings is more expensive than ints, bools, etc.
	//}

	////Where clause constraints
	//for (int i = 0; i < info->nConstraint; ++i)
	//{
	//	auto pConstraint = &info->aConstraint[i];
	//	pConstraint->iColumn; // Which column
	//	pConstraint->op; // operator
	//	pConstraint->usable; // true if this constraint is usable
	//}

	////Orders Fastore can support...
	////One column, any direction.
	////Two columns, if both are same direction and the second is RowId column.
	////With a contraint, the used constraint must be on the first column of the order.
	//bool hasOrder = false;
	//bool orderSupported = false;
	//double orderCost = 0.0;
	//if (info->nOrderBy == 0)
	//	orderSupported = true;
	//if (info->nOrderBy == 1 && (!hasConstraint || (info->aOrderBy[0].iColumn == constraintColumn)))
	//	orderSupported = true;
	//else if (info->nOrderBy == 2 && (!hasConstraint || (info->aOrderBy[0].iColumn == constraintColumn && info->aOrderBy[1].iColumn == 0 /* TODO: rowId column.. */)))
	//	orderSupported = true;

	//if (orderSupported && info->nOrderBy > 0)
	//	hasOrder = true;

	////Estimate order cost
	////Since we are already ordered, let's say it's always costless for now..
	//orderCost = 0.0;

	////Orderby clause (number of orders in order by)
	////for (int i = 0; i < info->nOrderBy; ++i)
	////{
	////	auto pOrder = &info->aOrderBy[i];
	////	pOrder->desc; // descending
	////	pOrder->iColumn; // Which column
	////}

	////Outputs...
	//for (int i = 0; i < info->nConstraint; ++i)
	//{
	//	if (i < 2 && constraintSupported)
	//	{
	//		info->aConstraintUsage[i].argvIndex = i; // if >0, constraint is part of argv to xFilter
	//		info->aConstraintUsage[i].omit = true; // suppress double check on rows received.
	//	}
	//	else
	//	{
	//		info->aConstraintUsage[i].argvIndex = -1; // if >0, constraint is part of argv to xFilter
	//		info->aConstraintUsage[i].omit = false; // suppress double check on rows received.
	//	}
	//}

	////Calculate cost...
	//double cost;
	//cost += hasOrder ? (orderSupported ? orderCost : 2000000.0 ) : 0; //We don't know if it being unordered adds any cost to the overall query. They query might not care.
	//cost += hasConstraint ? (constraintSupported ? constraintCost : 2000000.0 ) : 0; //Cost should be estimated from table size since the fewer constraints the larger the result.
	//info->idxNum = orderSupported && hasOrder ? (info->aOrderBy[0].desc ? 0 : 1) : -1; // Number used to identify the index  -- hijacking this to say asc/desc order(-1 = any, 0 = desc, 1 = asc)
	//info->idxStr; // String, possibly obtained from sqlite3_malloc
	//info->needToFreeIdxStr = false; //  Free idxStr using sqlite3_free() if true
	//info->orderByConsumed = hasOrder && orderSupported; //True if output is ordered..;
	//info->estimatedCost = cost; //  Estimated cost of using this index
}

client::RangeSet Table::getRange(client::Range& range, boost::optional<std::string>& startId)
{
	if (_transaction != NULL)
		return _transaction->GetRange(_columnIds, range, 500, startId);
	else
		return _connection->_database->GetRange(_columnIds, range, 500, startId);
}

void Table::update(int argc, sqlite3_value **argv, sqlite3_int64 *pRowid)
{
	if (sqlite3_value_type(argv[0]) != SQLITE_NULL)
	{
		sqlite3_int64 oldRowIdInt = sqlite3_value_int64(argv[0]);
		std::string oldRowId = Encoder<int>::Encode(oldRowIdInt);
		if (_transaction != NULL)
			_transaction->Exclude(_columnIds, oldRowId);
		else
			_connection->_database->Exclude(_columnIds, oldRowId);
	}

	//If it was a delete only, return.
	if (argc != _columnIds.size() + 2)
		return;

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
			{
				std::wstring toEncode((const wchar_t*)sqlite3_value_text16(pValue));
				value = Encoder<std::wstring>::Encode(toEncode);
			}

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