#pragma once
#include "stdafx.h"
#include "..\FastoreClient\Database.h"
#include "..\FastoreClient\Client.h"
#include "..\FastoreClient\Dictionary.h"
#include "..\FastoreClient\Encoder.h"
#include <vector>
#include <boost\assign\list_of.hpp>

using namespace boost::assign;
using namespace std;

class TestSetup{
public:
	boost::shared_ptr<fastore::client::Database> _database;
	std::vector<ColumnID> _columns;
	void createTableWithData();
};