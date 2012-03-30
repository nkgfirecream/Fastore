#include "ClientAPI.h"
#include "Table\dataset.h"

//IDataAccess
void IDataAccess::Exclude()
{

}

DataSet IDataAccess::GetRange()
{
	ColumnTypeVector ctv;
	TupleType tt(ctv);
	DataSet ds(tt, 1);
	return ds;
}

DataSet IDataAccess::GetRows()
{
	ColumnTypeVector ctv;
	TupleType tt(ctv);
	DataSet ds(tt, 1);
	return ds;
}

void* IDataAccess::Include()
{
	return NULL;
}

//ITransaction
void ITransaction::Dispose()
{

}

void ITransaction::Commit()
{

}

//ISession
ITransaction ISession::Begin(bool readIsolation, bool writeIsolation)
{
	return ITransaction();
}

void ISession::Dispose()
{

}

//IDatabase
TransactionID IDatabase::GetID()
{
	return TransactionID();
}

ISession IDatabase::Start()
{
	return ISession();
}

//IHostFactory
IHost HostFactory::Create(Topology topo)
{
	return IHost();
}

//Client
IDatabase Client::Connect(IHost host)
{
	return IDatabase();
}



