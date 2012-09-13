#include "Table.h"
#include "Dictionary.h"
#include "../FastoreCore/safe_cast.h"
#include "../FastoreClient/Dictionary.h"
#include "../FastoreClient/Encoder.h"
#include "../FastoreClient/Transaction.h"
#include <boost/assign/list_of.hpp>
#include "Utilities.h"
#include <sstream>

namespace module = fastore::module;
namespace client = fastore::client;
namespace communication = fastore::communication;

std::map<std::string, std::string> module::Table::fastoreTypesToSQLiteTypes;
std::map<std::string, std::string>  module::Table::sqliteTypesToFastoreTypes;
std::map<int, std::string>  module::Table::sqliteTypeIDToFastoreTypes;

module::Table::Table(module::Connection* connection, const std::string& name, const std::string& ddl) 
	: _connection(connection), _name(name),  _ddl(ddl) 
{}

void module::Table::begin()
{
	_transaction = _connection->_database->Begin(true, true);
}

void module::Table::sync()
{
	//Do nothing for now.. we may need to expose two stage transactions on the client.
}

void module::Table::commit()
{
	_transaction->Commit();
	_transaction.reset();
}

void module::Table::rollback()
{
	_transaction->Rollback();
	_transaction.reset();
}

//This code makes the assumption that the module has
//Already bootstrapped the Table of Tables.
void module::Table::create()
{
	ensureTable();
	ensureColumns();
	updateStats();
}

void module::Table::ensureTable()
{
	//See if the table already exists.
	client::RangeBound bound;
	bound.Bound = _name;
	bound.Inclusive = true;

	client::Range tableQuery;
	tableQuery.Ascending = true;
	tableQuery.ColumnID = module::Dictionary::TableName;
	tableQuery.End = bound;
	tableQuery.Start = bound;

	auto result = _connection->_database->GetRange(module::Dictionary::TableColumns, tableQuery, 1);
	
	if (result.Data.size() > 0)
	{
		_id = Encoder<communication::ColumnID>::Decode(result.Data[0].ID);
		//TODO: Test ddl and name to make sure they match;
	}
	else
	{
		_id = _connection->_generator->Generate(module::Dictionary::TableID);
		auto transaction =  _connection->_database->Begin(true, true);
		transaction->Include
		(
			module::Dictionary::TableColumns,
			client::Encoder<communication::ColumnID>::Crunch(_id),
			boost::assign::list_of<std::string>
			(client::Encoder<communication::ColumnID>::Crunch(_id))
			(_name)
			(_ddl)
		);
		transaction->Commit();
	}	
}

void module::Table::ensureColumns()
{
	//Gives us a valid set of column defs -- except for ids, which we need to determine.
	parseDDL();

	 // Pull a list of candidate pods
	 Range podQuery;
	podQuery.ColumnID = client::Dictionary::PodID;
	podQuery.Ascending = true;

	std::vector<ColumnID> podidv;
	podidv.push_back(client::Dictionary::PodID);

	////TODO: Gather pods - we may have more than 2000
    auto podIds = _connection->_database->GetRange(podidv, podQuery, 2000);
    if (podIds.Data.size() == 0)
        throw "FastoreModule can't create a new table. The hive has no pods. The hive must be initialized first.";


	//TODO: Determine key -- if it's more than one column, we need to create a surrogate.
	//For now, just assume a "hidden" key.

    // Start distributing columns on a random pod. Otherwise, we will always start on the first pod
    int nextPod = rand() % SAFE_CAST(int, (podIds.Data.size() - 1));

    //This is so we have quick access to all the ids (for queries). Otherwise, we have to iterate the 
    //TableVar Columns and pull the id each time.
    //List<int> columnIds = new List<int>();
    for (size_t i = 0; i < _columns.size(); i++)
	{
		auto combinedName = _name + "." + _columns[i].Name;

		// Attempt to find the column by name
		Range query;
		query.ColumnID = client::Dictionary::ColumnName;

		client::RangeBound bound;
		bound.Bound = combinedName;
		bound.Inclusive = true;
		query.Start = bound;
		query.End = bound;
		//Just pull one row. If any exist we have a problem.
		auto result = _connection->_database->GetRange(client::Dictionary::ColumnColumns, query, 1);

		if (result.Data.size() > 0)
		{
			int columnId = Encoder<int>::Decode(result.Data[i].ID);
			_columns[i].ColumnID = columnId;
			_columnIds.push_back(columnId);
		}
		else
		{
			int columnId = (int)_connection->_generator->Generate(client::Dictionary::ColumnID, module::Dictionary::MaxModuleColumnID + 1);			
			_columnIds.push_back(columnId);
			_columns[i].ColumnID = columnId;
			createColumn(_columns[i], combinedName, podIds, nextPod);
		}
	}
}

void module::Table::connect()
{
	//Try to locate all the columns (range over the columns table with "tablename.") and gather their ids
	//so that we can use them.
	//Try to locate rowId column if the column exists. 
}

void module::Table::drop()
{
	//TODO: drop module tables as well

	for (auto col : _columnIds)
	{
        // Pull a list of the current repos so we can drop them all.
        Range repoQuery;
        repoQuery.ColumnID = client::Dictionary::PodColumnColumnID;
        repoQuery.Ascending = true;
		client::RangeBound bound;
		bound.Inclusive = true;
		bound.Bound = client::Encoder<communication::ColumnID>::Encode(col);
        repoQuery.Start = bound;
        repoQuery.End = bound;

		auto repoIds = _connection->_database->GetRange(boost::assign::list_of<communication::ColumnID>(client::Dictionary::PodColumnColumnID), repoQuery, 2000);

		for (size_t i = 0; i < repoIds.Data.size(); i++)
        {
            _connection->_database->Exclude(client::Dictionary::PodColumnColumns, repoIds.Data[i].ID);
        }

		Range query;
        query.ColumnID = client::Dictionary::ColumnID;
        query.Start = bound;
        query.End = bound;

        auto columnExists = _connection->_database->GetRange(boost::assign::list_of<communication::ColumnID>(client::Dictionary::ColumnID), query, 2000);

        if (columnExists.Data.size() > 0)
        {
            _connection->_database->Exclude(client::Dictionary::ColumnColumns,  client::Encoder<communication::ColumnID>::Encode(col));
        }
	}
}

void module::Table::disconnect()
{
	//Do nothing for now...
}

void module::Table::createColumn(client::ColumnDef& column, std::string& combinedName, RangeSet& podIds, int nextPod)
{
	//TODO: Determine the storage pod - default, but let the user override -- we'll need to extend the sql to support this.
	auto podId = podIds.Data[nextPod++ % podIds.Data.size()].Values[0].value;

	//TODO: Make workers smart enough to create/instantiate a column within one transaction.
	//(They currently don't check for actions to perform until the end of the transaction, which means they may miss part of it currently)
	auto transaction =  _connection->_database->Begin(true, true);
	transaction->Include
	(
		module::Dictionary::TableColumnColumns,
		client::Encoder<communication::ColumnID>::Encode(_connection->_generator->Generate(module::Dictionary::TableColumnTableID)),
		boost::assign::list_of<std::string>
		(Encoder<communication::ColumnID>::Crunch(_id))
		(Encoder<communication::ColumnID>::Encode(column.ColumnID))
	);
	transaction->Commit();

	transaction =  _connection->_database->Begin(true, true);
	transaction->Include
	(
		client::Dictionary::ColumnColumns,
		client::Encoder<communication::ColumnID>::Encode(column.ColumnID),
		boost::assign::list_of<std::string>
		(client::Encoder<communication::ColumnID>::Encode(column.ColumnID))
		(combinedName)
		(column.Type)
		(column.IDType)
		(client::Encoder<client::BufferType_t>::Encode(column.BufferType))
		(client::Encoder<bool>::Encode(column.Required))
	);
	transaction->Commit();

	transaction = _connection->_database->Begin(true, true);
	transaction->Include
	(
		client::Dictionary::PodColumnColumns,
		client::Encoder<long long>::Encode(_connection->_generator->Generate(client::Dictionary::PodColumnPodID)),
		boost::assign::list_of<std::string>
		(podId)
		(client::Encoder<communication::ColumnID>::Encode(column.ColumnID))
	);
	transaction->Commit();		
}

char *
sqlite3_safe_malloc( size_t n )
{
	return reinterpret_cast<char*>( sqlite3_malloc(SAFE_CAST(int,n)) );
}

void module::Table::bestIndex(sqlite3_index_info* info)
{
	//Step 1. Group constraints by columns:
	//Key is column, Value is list of array indicies pointing to constraints.
	std::map<int,std::vector<int>> constraintMap;

	for (int i = 0; i < info->nConstraint; ++i)
	{
		//Skip unusable constraints
		if (!info->aConstraint[i].usable)
			continue;

		int col = info->aConstraint[i].iColumn;
		auto iter = constraintMap.find(col);

		if (iter != constraintMap.end())
		{
			iter->second.push_back(i);
		}
		else
		{
			std::vector<int> cons;
			cons.push_back(i);
			constraintMap.insert(std::pair<int,std::vector<int>>(col, cons));
		}
	}

	//Step 2. Weight constraint groups
	//We want to to weight towards columns that  have a low average number of keys per value, and also have a low total number of keys (it that right? All columns should have ~the same number of keys within a table..).
	//Weight -> column
	std::map<int64_t, int> weights;

	//Column -> supported
	std::map<int, bool> supported;
	for (auto iter = constraintMap.begin(); iter != constraintMap.end(); ++iter)
	{
		bool isSupported = true;

		//column
		int col = iter->first;

		//cost factor -- strings are a bit more expensive to compare/search than integers.
		double cost = _columns[col].Type == "String" || _columns[col].Type == "WString" ? 1.1 : 1;

		//Average ids per value
		int64_t avg = _stats[col].unique > 0 ? _stats[col].total / _stats[col].unique : 0;
		
		int64_t size = 100; // = 100% percent of the column for our purposes. Every constraint divides this by approx 2.
		if (iter->second.size() > 2)
			isSupported = false;
		else
		{
			for (size_t i = 0; i < iter->second.size(); ++i)
			{
				auto constraint = info->aConstraint[iter->second[i]];

				if (constraint.op == SQLITE_INDEX_CONSTRAINT_MATCH)
					isSupported = false;
				else if (constraint.op == SQLITE_INDEX_CONSTRAINT_EQ)
					size = 1;					
				else
					size = size / 2;
			}
		}
		// Arbitrarily huge number to bubble non-supported constraints to the top. 
		// That way we only need to check the lowest and see if it's supported. -- consider the overflow case...
		int64_t total = static_cast<int64_t>(static_cast<double>(size * avg) * cost * (isSupported ? 1 : 1000000000)); 

		weights[total] = col;
		supported[col] = isSupported;
	}

	//Step 3. If we have a supported constraint, use it
	bool useConstraint = constraintMap.size() > 0 && supported[weights.begin()->second];
	int whichColumn = weights.begin()->second;

	char* idxstr = NULL;

	if (useConstraint)
	{
		std::string params;
		auto vector = constraintMap[whichColumn];
		for (size_t i = 0 ; i < vector.size(); ++i)
		{
			info->aConstraintUsage[vector[i]].argvIndex = SAFE_CAST(int,i) + 1;
			info->aConstraintUsage[vector[i]].omit = true;
			params += info->aConstraint[vector[i]].op;
		}

		idxstr = sqlite3_safe_malloc(params.size() + 1);
		memcpy(idxstr, params.c_str(), params.size() + 1);		
	}

	//Step 4. Match the index to the constraint if possible, if not, see if order is usable by itself.
	bool useOrder = false;
	if (info->nOrderBy == 0 || (info->nOrderBy == 1 && !useConstraint) || (info->nOrderBy == 1 && info->aOrderBy[0].iColumn == whichColumn))
	{
		info->orderByConsumed = true;
		useOrder = true;
	}

	//Step 5. Estimate total cost
	double cost = 0;
	if (useConstraint)
		cost = static_cast<double>(weights.begin()->first);
	else if (useOrder)
		cost = static_cast<double>(_stats[info->aOrderBy[0].iColumn].total);
	else
		cost = static_cast<double>(_stats[0].total); //If no ordering, size of whole table -- pick a key column.

	//Step 6. Set remaining outputs.
	info->estimatedCost = cost;
	info->idxNum = useOrder ? (info->aOrderBy[0].desc ?  ~(info->aOrderBy[0].iColumn + 1) : (info->aOrderBy[0].iColumn + 1)) : 0; //TODO: Else should pick a required column.
	info->idxStr = idxstr;
	info->needToFreeIdxStr = true;
}

client::RangeSet module::Table::getRange(client::Range& range, const boost::optional<std::string>& startId)
{
	if (_transaction != NULL)
		return _transaction->GetRange(_columnIds, range, 500, startId);
	else
		return _connection->_database->GetRange(_columnIds, range, 500, startId);
}

void module::Table::update(int argc, sqlite3_value **argv, sqlite3_int64 *pRowid)
{
	//Update statistics every MAXOPERATIONS
	++_numoperations;
	_numoperations = _numoperations % MAXTABLEOPERATIONS;

	if(_numoperations == 0)
		updateStats();

	if (sqlite3_value_type(argv[0]) != SQLITE_NULL)
	{
		sqlite3_int64 oldRowIdInt = sqlite3_value_int64(argv[0]);
		std::string oldRowId = Encoder<sqlite3_int64>::Encode(oldRowIdInt);
		if (_transaction != NULL)
			_transaction->Exclude(_columnIds, oldRowId);
		else
			_connection->_database->Exclude(_columnIds, oldRowId);
	}

	//If it was a delete only, return.
	if (SAFE_CAST(size_t, argc) != _columnIds.size() + 2)
		return;

	sqlite3_int64 rowIdInt;
	if (sqlite3_value_type(argv[1]) != SQLITE_NULL)
	{
		rowIdInt = sqlite3_value_int64(argv[1]);
	}
	else
	{
		//TODO: generate on table Id
		rowIdInt = _connection->_generator->Generate(SAFE_CAST(int, _id));
		*pRowid = rowIdInt;
	}

	std::string rowid = Encoder<sqlite3_int64>::Encode(rowIdInt);

	std::vector<std::string> row;

	communication::ColumnIDs includedColumns;
	for (size_t i = 0; i < _columns.size(); ++i)
	{
		auto pValue = argv[i+2];
		if (sqlite3_value_type(pValue) != SQLITE_NULL)
		{
			std::string type = _columns[i].Type;

			std::string value;
			if (type == "Bool")
				value = Encoder<bool>::Encode(0 != sqlite3_value_int(pValue));
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
			includedColumns.push_back(_columns[i].ColumnID);
		}
		else if (_columns[i].Required)
		{
			throw "Got a NULL value on a required row!";
		}
	}	

	if (_transaction != NULL)
		_transaction->Include(includedColumns, rowid, row);
	else
		_connection->_database->Include(includedColumns, rowid, row);
}

void module::Table::updateStats()
{
	if (_transaction != NULL)
		_stats = _transaction->GetStatistics(_columnIds);
	else
		_connection->_database->GetStatistics(_columnIds);

	++_numoperations;
}

void module::Table::parseDDL()
{
	//Split on commas. Find columns definitions. Create defs based on definitions.
	//For now, assume all definitions are column defs.
	std::string col;
	std::istringstream reader(_ddl, std::istringstream::in);
	while (std::getline(reader, col, ','))
	{
		auto def = parseColumnDef(col);
		_columns.push_back(def);
	}
}

client::ColumnDef module::Table::parseColumnDef(std::string text)
{
	client::ColumnDef result;
	std::istringstream reader(text, std::istringstream::in);
	if (!std::getline(reader, result.Name, ' ')) 
		std::runtime_error("Missing column name");

	std::string type;
	if (!std::getline(reader, type, ' '))
		result.Type = "String";
	else
		result.Type = SQLiteTypeToFastoreType(type);

	auto stringText = std::string(text);
	//TODO: When should we use an identity buffer? Primary key?
	result.BufferType =	insensitiveStrPos(stringText, std::string("primary")) >= 0 ? 
	    client::BufferType_t::Identity : 
	    	insensitiveStrPos(stringText, std::string("unique")) >= 0 ? 
	    		client::BufferType_t::Unique : client::BufferType_t::Multi;
	result.Required = insensitiveStrPos(stringText, std::string("not null")) >= 0 || 
	                  insensitiveStrPos(stringText, std::string("primary")) >= 0;

	result.IDType = "Int";
	return result;
}

void module::Table::EnsureFastoreTypeMaps()
{
	if (fastoreTypesToSQLiteTypes.size() == 0)
	{
		fastoreTypesToSQLiteTypes["WString"] = "nvarchar";
		fastoreTypesToSQLiteTypes["String"] = "varchar";
		fastoreTypesToSQLiteTypes["Int"] = "int";
		fastoreTypesToSQLiteTypes["Long"] = "bigint";
		fastoreTypesToSQLiteTypes["Bool"] = "int";
	}

	if (sqliteTypesToFastoreTypes.size() == 0)
	{
		sqliteTypesToFastoreTypes["nvarchar"] = "WString";
		sqliteTypesToFastoreTypes["varchar"] = "String";
		sqliteTypesToFastoreTypes["int"] = "Int";
		sqliteTypesToFastoreTypes["bigint"] = "Long";
		//TODO: Must add special handling for bool. Either don't use it in Fastore, or check column def.
		//(actually, can we even define a bool column in sqlite?)
		//sqliteTypesToFastoreTypes["int"] = "Bool";
	}

	if (sqliteTypesToFastoreTypes.size() == 0)
	{
		sqliteTypeIDToFastoreTypes[SQLITE_INTEGER] = "Long";
		sqliteTypeIDToFastoreTypes[SQLITE_TEXT] = "String";
	}
}

std::string module::Table::FastoreTypeToSQLiteType(const std::string &fastoreType)
{
	EnsureFastoreTypeMaps();
	auto result = fastoreTypesToSQLiteTypes.find(fastoreType);
	if (result == fastoreTypesToSQLiteTypes.end())
	{
		std::ostringstream message;
		message << "Unknown type '" << fastoreType << "'.";
		std::runtime_error(message.str());
	}
	return result->second;
}

std::string module::Table::SQLiteTypeToFastoreType(const std::string &SQLiteType)
{
	EnsureFastoreTypeMaps();
	auto result = sqliteTypesToFastoreTypes.find(SQLiteType);
	if (result == sqliteTypesToFastoreTypes.end())
	{
		std::ostringstream message;
		message << "Unknown type '" << SQLiteType << "'.";
		std::runtime_error(message.str());
	}
	return result->second;			
}
