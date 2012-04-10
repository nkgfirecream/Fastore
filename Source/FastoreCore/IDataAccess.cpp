#include "IDataAccess.h"
#include "Table\dataset.h"

//IDataAccess
void IDataAccess::Exclude(void* rowIds[], int columns[], bool isPicky)
{

}

DataSet IDataAccess::GetRange(int columns[], Range range)
{
	ColumnTypeVector ctv;
	TupleType tt(ctv);
	DataSet ds(tt, 1);
	return ds;
}

DataSet IDataAccess::GetRows(void* rowdIds[], int columns[])
{
	ColumnTypeVector ctv;
	TupleType tt(ctv);
	DataSet ds(tt, 1);
	return ds;
}

void* IDataAccess::Include(void* rowIds[], int columns[] , bool isPicky)
{
	return NULL;
}
