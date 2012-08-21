#include "Table.h"

using namespace fastore::module;

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
	
}

void Table::connect()
{

}

void Table::drop()
{

}

void Table::disconnect()
{

}