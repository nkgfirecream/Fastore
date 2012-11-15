#include "Table.h"
#include "Dictionary.h"
//#include "../FastoreCore/safe_cast.h"
#include "../FastoreCommon/Type/Standardtypes.h"
#include "../FastoreClient/Dictionary.h"
#include "../FastoreClient/Encoder.h"
#include "../FastoreClient/Transaction.h"
#include <boost/assign/list_of.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Utilities.h"
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <math.h>

namespace module = fastore::module;
namespace client = fastore::client;
namespace communication = fastore::communication;

std::map<std::string, ScalarType, LexCompare>  module::Table::declaredTypeToFastoreType;
std::map<std::string, int, LexCompare>  module::Table::declaredTypeToSQLiteTypeID;

module::Table::Table(module::Connection* connection, const std::string& name, const std::string& ddl) 
	: _connection(connection), _name(name),  _ddl(ddl), _rowIDIndex(-1), _numoperations(0)
{
		EnsureFastoreTypeMaps();
}

void module::Table::EnsureFastoreTypeMaps()
{
	if (declaredTypeToFastoreType.size() == 0)
	{
		declaredTypeToFastoreType["varchar"] = standardtypes::String;
		declaredTypeToFastoreType["int"] = standardtypes::Long;
		declaredTypeToFastoreType["float"] = standardtypes::Double;
		declaredTypeToFastoreType["date"] = standardtypes::Long;
		declaredTypeToFastoreType["datetime"] = standardtypes::Long;
	}

	if (declaredTypeToSQLiteTypeID.size() == 0)
	{
		declaredTypeToSQLiteTypeID["varchar"] = SQLITE_TEXT;
		declaredTypeToSQLiteTypeID["int"] = SQLITE_INTEGER;
		declaredTypeToSQLiteTypeID["float"] = SQLITE_FLOAT;
		declaredTypeToSQLiteTypeID["date"] = SQLITE_INTEGER;
		declaredTypeToSQLiteTypeID["datetime"] = SQLITE_INTEGER;
	}
}

int module::Table::begin()
{
	_transaction = _connection->_database->Begin(true, true);
	return SQLITE_OK;
}

int module::Table::sync()
{
	//Do nothing for now.. we may need to expose two stage transactions on the client.
	return SQLITE_OK;
}

int module::Table::commit()
{
	if (_transaction != NULL)
	{
		_transaction->Commit();
		_transaction.reset();
	}
	return SQLITE_OK;
}

int module::Table::rollback()
{
	if (_transaction != NULL)
	{
		_transaction->Rollback();
		_transaction.reset();
	}
	return SQLITE_OK;
}

//This code makes the assumption that the module has
//Already bootstrapped the Table of Tables.
int module::Table::create()
{
	ensureTable();
	ensureColumns();
	updateStats();

	return SQLITE_OK;
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
		_id = _connection->_generator->Generate(module::Dictionary::TableID, module::Dictionary::MaxModuleColumnID + 1);
		auto transaction =  _connection->_database->Begin(true, true);
		transaction->Include
		(
			module::Dictionary::TableColumns,
			client::Encoder<communication::ColumnID>::Encode(_id),
			boost::assign::list_of<std::string>
			(client::Encoder<communication::ColumnID>::Encode(_id))
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

	//if we didn't get any columns from the dll, abort the operation. (Rollback the transaction.. hopefully).
	//Since this is all happening outside the context of a SQLite transaction, we'll need a way to undo what we've
	//already done. One option that comes to mind is to start our own transaction internally. How does that affect
	//key generation, for example, key generation? That already occurred in its own transaction.
	if (_columns.size() == 0)
		throw "No columns found in table defintion";

	//Pick a column to use as rowIds if available. -1 = no column available.
	determineRowIDColumn();

	 // Pull a list of candidate pods
	 Range podQuery;
	podQuery.ColumnID = client::Dictionary::PodID;
	podQuery.Ascending = true;

	std::vector<ColumnID> podidv;
	podidv.push_back(client::Dictionary::PodID);

	////TODO: Gather pods - we may have more than 2000
    auto podIds = _connection->_database->GetRange(podidv, podQuery, 2000);
    if (podIds.Data.size() == 0)
        throw "FastoreModule can't create a new table. The hive has no pods. The hive must be initialized first."; //TODO: Map this to an error code?


	//TODO: Determine key -- if it's more than one column, we need to create a surrogate.
	//For now, just assume a "hidden" key.

    // Start distributing columns on a random pod. Otherwise, we will always start on the first pod
    int nextPod = 0; //rand() % SAFE_CAST(int,(podIds.Data.size() - (podIds.Data.size() == 1? 0 : 1)));

    for (size_t i = 0; i < _columns.size(); i++)
	{
		auto combinedName = _name + "." + _columns[i].Name;

		// Attempt to find the column by name
		Range query;
		query.ColumnID = client::Dictionary::ColumnName;
		query.Ascending = true;

		client::RangeBound bound;
		bound.Bound = combinedName;
		bound.Inclusive = true;
		query.Start = bound;
		query.End = bound;
		auto result = _connection->_database->GetRange(client::Dictionary::ColumnColumns, query, 1);

		if (result.Data.size() > 0)
		{
			int columnId = Encoder<int>::Decode(result.Data[0].ID);
			_columns[i].ColumnID = columnId;
			_columnIds.push_back(columnId);
		}
		else
		{
			//TODO: Determine the storage pod - default, but let the user override -- we'll need to extend the sql to support this.
			auto podId = podIds.Data.at(nextPod++ % podIds.Data.size()).Values[0].value;

			int columnId = (int)_connection->_generator->Generate(client::Dictionary::ColumnID, module::Dictionary::MaxModuleColumnID + 1);			
			_columnIds.push_back(columnId);
			_columns[i].ColumnID = columnId;
			createColumn(_columns[i], combinedName, podIds, podId);
		}
	}
}

int module::Table::connect()
{
	//Try to locate all the columns (range over the columns table with "tablename.") and gather their ids
	//so that we can use them.
	//Try to locate rowId column if the column exists. 

	return SQLITE_ERROR;
}

int module::Table::drop()
{
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

	//Drop TableColumns entry
	Range tableColumnQuery;
	tableColumnQuery.ColumnID = module::Dictionary::TableColumnTableID;
	tableColumnQuery.Ascending = true;
	client::RangeBound bound;
	bound.Inclusive = true;
	bound.Bound = client::Encoder<communication::ColumnID>::Encode(_id);
	tableColumnQuery.Start = bound;
	tableColumnQuery.End = bound;

	auto tableColumnsResult = _connection->_database->GetRange(module::Dictionary::TableColumnColumns, tableColumnQuery, 2000);

	for (size_t i = 0; i < tableColumnsResult.Data.size(); i++)
	{
		 _connection->_database->Exclude(module::Dictionary::TableColumnColumns, tableColumnsResult.Data[i].ID);
	}

	//Drop Table Table entry
	_connection->_database->Exclude(module::Dictionary::TableColumns, client::Encoder<communication::ColumnID>::Encode(_id));

	return SQLITE_OK;
}

int module::Table::disconnect()
{
	//Do nothing for now...
	return SQLITE_ERROR;
}

void module::Table::createColumn(ColumnDef& column, std::string& combinedName, RangeSet& podIds, std::string& podId)
{

	//TODO: Make workers smart enough to create/instantiate a column within one transaction.
	//(They currently don't check for actions to perform until the end of the transaction, which means they may miss part of it currently)
	auto transaction =  _connection->_database->Begin(true, true);
	transaction->Include
	(
		client::Dictionary::ColumnColumns,
		client::Encoder<communication::ColumnID>::Encode(column.ColumnID),
		boost::assign::list_of<std::string>
		(client::Encoder<communication::ColumnID>::Encode(column.ColumnID))
		(combinedName)
		(column.ValueType.Name)
		(column.RowIDType.Name)
		(client::Encoder<BufferType_t>::Encode(column.BufferType))
		(client::Encoder<bool>::Encode(column.Required))
	);
	transaction->Include
	(
		client::Dictionary::PodColumnColumns,
		client::Encoder<int64_t>::Encode(_connection->_generator->Generate(client::Dictionary::PodColumnPodID)),
		boost::assign::list_of<std::string>
		(podId)
		(client::Encoder<communication::ColumnID>::Encode(column.ColumnID))
	);
	transaction->Include
	(
		module::Dictionary::TableColumnColumns,
		client::Encoder<communication::ColumnID>::Encode(_connection->_generator->Generate(module::Dictionary::TableColumnTableID)),
		boost::assign::list_of<std::string>
		(Encoder<communication::ColumnID>::Encode(_id))
		(Encoder<communication::ColumnID>::Encode(column.ColumnID))
	);
	transaction->Commit();		
}

char *sqlite3_safe_malloc( size_t n )
{
	return reinterpret_cast<char*>( sqlite3_malloc(SAFE_CAST(int,n)) );
}

int module::Table::bestIndex(sqlite3_index_info* info, double* numRows, int numIterations)
{
	//TODO: Do some real testing with real data and see what happens.
	//TODO: Fix disjoint constraints (>= 4 && <= 2). We say we support it, but we don't (see next line).
	//WRT disjoint constraints, this is now fixed in the cursor. It checks the constraints,
	//and if it recognizes it will pull an empty set, simply pull nothing. The reason for this
	//is that SQLite apparently does not make that optimization, so will revert to pulling every
	//row and checking the constraint against the row.

	//TODO: In the case of multiples (for example, two >=,>= or whatever) we can support it and figure out which one to use in filter.
	//std::cout << std::endl << "Table: " << _name << std::endl;
	//std::cout << "Number of constraints: " << info->nConstraint << std::endl;
	//for (int i = 0; i < info->nConstraint; ++i)
	//{
	//	auto c = info->aConstraint[i];
	//	std::cout << "Cons# " << i;// << std::endl;
	//	std::cout << "\tUse: " << (c.usable ? "True" : "False");
	//	std::cout << "\tCol: " << _columns[c.iColumn].Name;
	//	std::cout << "\tOp: ";
	//	switch(c.op)
	//	{
	//		case SQLITE_INDEX_CONSTRAINT_EQ:
	//			std::cout << "=";
	//			break;
	//		case SQLITE_INDEX_CONSTRAINT_GE:
	//			std::cout << ">=";
	//			break;
	//		case SQLITE_INDEX_CONSTRAINT_GT:
	//			std::cout << ">";
	//			break;
	//		case SQLITE_INDEX_CONSTRAINT_LE:
	//			std::cout << "<=";
	//			break;
	//		case SQLITE_INDEX_CONSTRAINT_LT:
	//			std::cout << "<";
	//			break;
	//		case SQLITE_INDEX_CONSTRAINT_MATCH:
	//			std::cout << "match";
	//			break;
	//		default:
	//			std::cout << "unknown";
	//			break;
	//	}

	//	std::cout << std::endl;
	//}

	//std::cout << "Number of orders: " << info->nOrderBy << std::endl;
	//for (int i = 0; i < info->nOrderBy; ++i)
	//{
	//	auto o = info->aOrderBy[i];
	//	std::cout << "Order# " << i;// << std::endl;
	//	std::cout << "\tCol: " << _columns[o.iColumn].Name; //<< std::endl;
	//	std::cout << "\tAsc: " << (o.desc ? "False" : "True");// << std::endl;
	//	std::cout << std::endl;
	//}



	//Step 1. Group constraints by columns:
	//Key is column, Value is list of array indicies pointing to constraints.
	std::map<int,std::vector<int>> constraintByColumn;

	for (int i = 0; i < info->nConstraint; ++i)
	{
		//Skip unusable constraints
		if (!info->aConstraint[i].usable)
			continue;

		int col = info->aConstraint[i].iColumn;
		auto iter = constraintByColumn.find(col);

		if (iter != constraintByColumn.end())
		{
			iter->second.push_back(i);
		}
		else
		{
			std::vector<int> cons;
			cons.push_back(i);
			constraintByColumn.insert(std::pair<int,std::vector<int>>(col, cons));
		}
	}

	//Step 2. Weight constraint groups
	//We want to to weight towards columns that  have a low average number of keys per value, and also have a low total number of rowIds
	//(is that right? All columns should have ~the same number of rowIds within a table..).

	//Weight -> column
	//The std::map is guaranteed to be ordered. We want to do this so that we find the least cost easily (it'll be the first key in the map).
	std::map<double, int> weights;

	for (auto column = constraintByColumn.begin(); column != constraintByColumn.end(); ++column)
	{
		bool isSupported = true;

		//column
		int colIndex = column->first;		

		//Average ids per value
		double avg = _stats[colIndex].unique > 0 ? double(_stats[colIndex].total) / double(_stats[colIndex].unique) : 1;
		
		double approxPercent = 1; // = 100% percent of the column for our purposes. Every constraint divides this by approx 2.
		if (column->second.size() > 2)
			isSupported = false;
		else
		{
			for (size_t i = 0; i < column->second.size(); ++i)
			{
				auto constraint = info->aConstraint[column->second[i]];

				if (constraint.op == SQLITE_INDEX_CONSTRAINT_MATCH)
					isSupported = false;
				else if (constraint.op == SQLITE_INDEX_CONSTRAINT_EQ)
					approxPercent = avg / (_stats[colIndex].total > 0 ? _stats[colIndex].total : 1);					
				else
					approxPercent = approxPercent / 2;
			}
		}
		
		if (isSupported)
		{
			//Total cost = percentage of rowIds * number of rowIds * cost per rowId
			double totalRows = approxPercent * _stats[colIndex].total;
			//double totalCost = totalCost * costPerRow(colIndex); //This is just used to favor
			weights[totalRows] = colIndex;
		}
	}

	//Step 3. If we have a supported constraint, use it and setup the idx string to pass to filter.
	bool useConstraint = weights.size() > 0;
	int whichColumn = 0;
	char* idxstr = NULL;

	if (useConstraint)
	{
		//Which column the constraint is on
		whichColumn = weights.begin()->second;

		std::string params;
		auto constraintIndexes = constraintByColumn[whichColumn];
		for (size_t i = 0 ; i < constraintIndexes.size(); ++i)
		{
			auto constraintIndex = constraintIndexes[i];
			info->aConstraintUsage[constraintIndex].argvIndex = int(i) + 1;
			info->aConstraintUsage[constraintIndex].omit = true;
			params += info->aConstraint[constraintIndex].op;
		}

		idxstr = sqlite3_safe_malloc(params.size() + 1);
		memcpy(idxstr, params.c_str(), params.size() + 1);
		info->idxStr = idxstr;
		info->needToFreeIdxStr = true;
	}

	//Step 4. Match the columns of the order to the constraint column if possible. If not, see if order is usable by itself.
	//If an order is more than one column, we can't support it
	bool useOrder = false;
	if ((info->nOrderBy == 1 && !useConstraint) || (info->nOrderBy == 1 && info->aOrderBy[0].iColumn == whichColumn))
	{
		info->orderByConsumed = true;
		useOrder = true;
		info->idxNum = (info->aOrderBy[0].desc ?  ~(info->aOrderBy[0].iColumn + 1) : (info->aOrderBy[0].iColumn + 1));
	}
	else if (useConstraint)
		info->idxNum =  whichColumn + 1;
	else
		info->idxNum = 0;

	//Step 5. Estimate total cost
	if (useConstraint)
	{
		*numRows = weights.begin()->first;
	}
	else if (useOrder)
	{
		*numRows = (double)_stats[info->aOrderBy[0].iColumn].total;
	}
	else
	{
		*numRows = (double)maxColTot();
	}//If no ordering, size of whole table -- pick a key column. We simulate this for now by picking the column with the highest total.

	info->estimatedCost = (*numRows) + (numIterations * (double)QUERYOVERHEAD); 

	//if (useConstraint)
	//{
	//	std::cout << "Constraint(s) supported on column: " << _columns[whichColumn].Name << std::endl;
	//}
	//else
	//{
	//	std::cout << "Constraint not supported" << std::endl;
	//}

	//if (useOrder)
	//{
	//	std::cout << "Order supported on column: " << _columns[whichColumn].Name << std::endl;
	//}
	//else
	//{
	//	std::cout << "Order not supported" << std::endl;
	//}

	//std::cout << "Estimated cost: " << info->estimatedCost << std::endl;
	//std::cout << "Estimated #rows: " << *numRows << std::endl << std::endl;

	return SQLITE_OK;
}

double module::Table::costPerRow(int columnIndex)
{
	return _columns[columnIndex].ValueType.Name == "String" || _columns[columnIndex].ValueType.Name == "WString" ? 1.1 : 1;
}

client::RangeSet module::Table::getRange(client::Range& range, const boost::optional<std::string>& startId)
{
	if (_transaction != NULL)
		return _transaction->GetRange(_columnIds, range, ROWSPERQUERY, startId);
	else
		return _connection->_database->GetRange(_columnIds, range, ROWSPERQUERY, startId);
}

int module::Table::update(int argc, sqlite3_value **argv, sqlite3_int64 *pRowid)
{
	//Update statistics every MAXOPERATIONS
	++_numoperations;
	_numoperations %= MAXTABLEOPERATIONS;

	if(_numoperations == 0)
		updateStats();


	//An update for us is the same as a delete + insert, so treat the first operation the same, whether or not an insert is present
	if (sqlite3_value_type(argv[0]) != SQLITE_NULL)
	{
		sqlite3_int64 oldRowIdInt = sqlite3_value_int64(argv[0]);
		std::string oldRowId = Encoder<sqlite3_int64>::Encode(oldRowIdInt);
		if (_transaction != NULL)
			_transaction->Exclude(_columnIds, oldRowId);
		else
			_connection->_database->Exclude(_columnIds, oldRowId);
	}

	//delete only, return
	if (argc == 1)
		return SQLITE_OK;


	//Find our row Id
	sqlite3_int64 rowIdInt;
	if (sqlite3_value_type(argv[1]) != SQLITE_NULL)
	{
		//SQLite supplied one
		rowIdInt = sqlite3_value_int64(argv[1]);
	}
	else if (_rowIDIndex != -1)
	{
		//We are using an identity column as a rowId.
		rowIdInt = sqlite3_value_int64(argv[2 + _rowIDIndex]); //First two values are delete and insert rowIds, actually row values follow
	}
	else
	{
		//Generate a new id.
		rowIdInt = _connection->_generator->Generate(_id);
	}

	//Set the pointer so SQLite knows what rowID we've picked
	*pRowid = rowIdInt;

	std::string rowid = Encoder<sqlite3_int64>::Encode(rowIdInt);

	std::vector<std::string> row;
	communication::ColumnIDs includedColumns;

	int result = convertValues(argv, includedColumns, row);
	if (result != SQLITE_OK)
		return result;

	if (_transaction != NULL)
		_transaction->Include(includedColumns, rowid, row);
	else
		_connection->_database->Include(includedColumns, rowid, row);

	return SQLITE_OK;
}

int module::Table::convertValues(sqlite3_value **argv, communication::ColumnIDs& columns, std::vector<std::string>& row)
{
	for (size_t i = 0; i < _columns.size(); ++i)
	{
		auto pValue = argv[i+2];
		if (sqlite3_value_type(pValue) != SQLITE_NULL)
		{
			std::string type = _declaredTypes[i];
			std::string value;

			int result = tryConvertValue(pValue, type, value);
			if (result != SQLITE_OK)
				return result;					

			row.push_back(value);
			columns.push_back(_columns[i].ColumnID);
		}
		else if (_columns[i].Required)
		{
			return SQLITE_CONSTRAINT;
		}
	}	

	return SQLITE_OK;
}

int module::Table::tryConvertValue(sqlite3_value* pValue, std::string& declaredType, std::string& out)
{	
	int datatype = sqlite3_value_type(pValue);
	int desiredType = declaredTypeToSQLiteTypeID[declaredType];

	//If the types match OR the desired type is string, just dump the data as is (this means blobs get dumped into strings as well).
	if (datatype == desiredType || desiredType == SQLITE_TEXT)
	{	
		encodeSQLiteValue(pValue, desiredType, out);
	}
	//Try a date time conversion.
	else if (strcasecmp(declaredType.c_str(), "date") == 0 || strcasecmp(declaredType.c_str(), "datetime") == 0)
	{
		static boost::posix_time::ptime start = boost::posix_time::from_time_t(0);

		std::string timestring = std::string((const char *)sqlite3_value_text(pValue));
		//TODO: The boost library does handle string->date conversions correctly unless are fully formatted (e.g. YYYY-MM-DD HH:MM:SS).
		if (timestring.find(':') == string::npos)
			timestring += " 00:00:00";
		boost::posix_time::ptime t(boost::posix_time::time_from_string(timestring));					
		boost::posix_time::time_duration dur = t - start;
		time_t epoch = dur.total_seconds();
		out = Encoder<int64_t>::Encode(epoch);
	}
	//Try a lossless conversion
	else if (desiredType == SQLITE_INTEGER || desiredType == SQLITE_FLOAT)
	{
		//TODO: This doesn't always seems to handle numbers entered as strings correctly, so investigate that...
		int result = sqlite3_value_numeric_type(pValue);
		//if (result != desiredType)
		// return SQLITE_MISMATCH;
		encodeSQLiteValue(pValue, desiredType, out);
	}		
	else
	{
		return SQLITE_MISMATCH;
	}				

	return SQLITE_OK;
}

void module::Table::encodeSQLiteValue(sqlite3_value* pValue, int type, std::string& out)
{
	switch(type)
	{
		case SQLITE_TEXT:
			out = std::string((const char *)sqlite3_value_text(pValue));
			break;
		case SQLITE_INTEGER:
			out = Encoder<int64_t>::Encode(sqlite3_value_int64(pValue));
			break;
		case SQLITE_FLOAT:
			out = Encoder<double>::Encode(sqlite3_value_double(pValue));
			break;
		default:
			throw "Unrecognized type!";
			break;
	}
}

void module::Table::updateStats()
{
	if (_transaction != NULL)
		_stats = _transaction->GetStatistics(_columnIds);
	else
		_stats = _connection->_database->GetStatistics(_columnIds);
}

void module::Table::parseDDL()
{
	//Split on commas. Find columns definitions. Create defs based on definitions.
	//For now, assume all definitions are column defs.
	std::string col;
	std::istringstream reader(_ddl, std::istringstream::in);
	bool hasIdentity = false;
	while (std::getline(reader, col, ','))
	{
		bool isDef = false;
		auto def = parseColumnDef(col, isDef);
		//Crappy bespoke parser for the time being... constraints will be here just like column definitions
		if (isDef)
		{
			//Ensure there is only one Identity Column per table
			if (def.BufferType == BufferType_t::Identity && hasIdentity)
				def.BufferType = BufferType_t::Unique;
			else if (def.BufferType == BufferType_t::Identity)
				hasIdentity = true;

			_columns.push_back(def);
		}
		else
			break; //Assume all column definitions are at the beginning of the table definition
	}
}

ColumnDef module::Table::parseColumnDef(std::string text, bool& isDef)
{
	ColumnDef result;
	std::istringstream reader(text, std::istringstream::in);
	if (!std::getline(reader, result.Name, ' ')) 
		std::runtime_error("Missing column name");

	if (result.Name == "")
	{
		isDef = false;
		return result;
	}

	std::string type;
	if (!std::getline(reader, type, ' '))
	{
		isDef = false;
		return result;
	}
	else
	{
		//More crappy parser-ness. If we don't recongize a type, it'll blow up!
		_declaredTypes.push_back(type);
		result.ValueType = declaredTypeToFastoreType[type];
	}

	isDef = true;
	auto stringText = std::string(text);
	//TODO: When should we use an identity buffer? Primary key?
	result.BufferType =	insensitiveStrPos(stringText, std::string("primary")) >= 0 ? 
	    BufferType_t::Identity : 
	    	insensitiveStrPos(stringText, std::string("unique")) >= 0 ? 
	    		BufferType_t::Unique : BufferType_t::Multi;
	result.Required = insensitiveStrPos(stringText, std::string("not null")) >= 0 || 
	                  insensitiveStrPos(stringText, std::string("primary key")) >= 0;

	result.RowIDType = standardtypes::Long;
	return result;
}

void module::Table::determineRowIDColumn()
{
	//Qualities of a rowId column -- Identity, Required, Integer -- we don't particularly care if it's marked as key or not.
	//However... That suggests only one identity column per table.. so it is a key...
	for (size_t i = 0; i < _columns.size(); i++)
	{
		ColumnDef col = _columns[i];

		if (col.Required && col.BufferType == BufferType_t::Identity && col.ValueType.Name == "Long")
		{
			_rowIDIndex = i;
			break;
		}
	}
}

int64_t module::Table::maxColTot()
{
	if (_rowIDIndex != -1)
	{
		return _stats[_rowIDIndex].total;
	}
	else
	{
		int64_t max = 0;
		for (auto col = _stats.begin(), end = _stats.end(); col != end; ++col)
		{
			if (col->total > max)
				max = col->total;
		}

		return max;
	}
}
