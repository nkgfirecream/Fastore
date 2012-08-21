#include "stdafx.h"
#include "..\FastoreClient\ServiceAddress.h"
#include "..\FastoreClient\Database.h"
#include "..\FastoreClient\Client.h"
#include "..\FastoreClient\Dictionary.h"
#include "..\FastoreClient\Encoder.h"
#include <vector>
#include <boost\assign\list_of.hpp>

using namespace fastore::client;
using namespace boost::assign;
using namespace std;

class TestSetup{
	boost::shared_ptr<fastore::client::Database> _database;
public:
	void createTableWithData();
};