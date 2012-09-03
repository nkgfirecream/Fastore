#pragma once
#include <map>
#include "ColumnDef.h"

using namespace fastore::client;

typedef std::map<int, fastore::client::ColumnDef> Schema;
