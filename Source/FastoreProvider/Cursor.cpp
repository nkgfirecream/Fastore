#include "Cursor.h"

using namespace std;

fastore::provider::Cursor::Cursor(IDataAccess *dataAccess, const string &sql) : _dataAccess(dataAccess)
{
}
