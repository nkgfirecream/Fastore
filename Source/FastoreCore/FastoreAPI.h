#include "typedefs.h"
#include "TransactionID.h"
#include "Table\dataset.h"

class IDataAccess
{
	public:
		DataSet GetRange(/*Columns, [range], [sorting]*/);
		DataSet GetRows(/* RowIds[], Columns, Sorting*/);
		void* Include(/* row, columns, isPicky*/);
		void Exclude (/* rowid[], columns, ispicky */);
		//void Exclude(range, columns, isPicky);
};

class ITransaction : public IDataAccess
{
	public:
		void Dispose();
		void Commit();
};

class ISession : public IDataAccess
{
	public:
		void Dispose();
		ITransaction Begin(bool ReadIsolation, bool WriteIsolation);
};

class IDatabase
{
	public:
		ISession Start();
		TransactionID GetID(/* Token */);
		//Low priority
		//void Topology();
		//ILock GetLock(/*name, mode */);
};


class IHost
{
	//HostInformation
};


class Topology
{
	//Topology Information
};


class HostFactory
{
	public:
		IHost Create(Topology topo);
		//Lower Priority
		//IHost Connect(address);
		
};

class Client
{
	public:
		IDatabase Connect(IHost host);
};

//LowPriority
/*
class ILock
{

};
*/







