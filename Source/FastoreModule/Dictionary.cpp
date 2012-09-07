#include "Dictionary.h"

#define COUNTOF(a) (sizeof(a)/sizeof(*a))

using namespace fastore::module;
using fastore::communication::ColumnID;

const ColumnID Dictionary::MaxModuleColumnID(29999);

const ColumnID Dictionary::TableID(20000);
const ColumnID Dictionary::TableName(20001);
const ColumnID Dictionary::TableDDL(20002);
const ColumnID Dictionary::_TableColumns[] = { TableID, TableName, TableDDL };
const ColumnIDs  Dictionary::TableColumns(_TableColumns, _TableColumns + COUNTOF(_TableColumns));


const ColumnID Dictionary::TableColumnTableID(20100);
const ColumnID Dictionary::TableColumnColumnID(20101);
const ColumnID Dictionary::_TableColumnColumns[] = { TableColumnTableID, TableColumnColumnID };
const ColumnIDs  Dictionary::TableColumnColumns(_TableColumnColumns, _TableColumnColumns + COUNTOF(_TableColumnColumns));
