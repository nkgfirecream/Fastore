#pragma once

#include "Cursor.h"

using namespace fastore::module;

Cursor::Cursor(fastore::module::Database* database) : _database(database) { }