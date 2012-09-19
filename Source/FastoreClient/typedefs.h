#pragma once
#include <map>
#include "ColumnDef.h"

using namespace fastore::client;

typedef std::map<ColumnID, fastore::client::ColumnDef> Schema;
