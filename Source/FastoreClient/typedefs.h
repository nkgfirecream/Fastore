#pragma once
#include <map>
#include "..\FastoreCommon\Buffer\ColumnDef.h"

using namespace fastore::client;

typedef std::map<ColumnID, ColumnDef> Schema;
