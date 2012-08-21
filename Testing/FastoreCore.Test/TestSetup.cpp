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

//void Include(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row);
        
void TestSetup::createTableWithData()
{
	//connect
	ServiceAddress address;
	address.setName("localhost");
	address.setPort(8064);

	std::vector<ServiceAddress> addresses;
	addresses.push_back(address);

	std::vector<ColumnID> _columns = list_of<ColumnID>(10000)(10001)(10002)(10003)(10004)(10005);

	auto _database = Client::Connect(addresses);

	//create schema
	_database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[0]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[0]))("ID")("Int")("Int")(Encoder<BufferType>::Encode(BufferType::Identity))(Encoder<bool>::Encode(true)));
	_database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[1]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[1]))("Given")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    _database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[2]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[2]))("Surname")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    _database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[3]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[3]))("Gender")("Bool")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    _database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[4]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[4]))("BirthDate")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    _database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[5]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[5]))("BirthPlace")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));

	//add data
	/*_database->Include(cols, 1, new object[] { 1, "Joe", "Shmoe", true, "5/26/1980", "Antarctica" });
	_database->Include(cols, 2, new object[] { 2, "Ann", "Shmoe", false, "4/20/1981", "Denver" });
	_database->Include(cols, 3, new object[] { 3, "Sarah", "Silverman", false, "11/10/1976", "Chicago" });
	_database->Include(cols, 4, new object[] { 4, "Bob", "Newhart", true, "12/2/1970", "Paris" });
	_database->Include(cols, 5, new object[] { 5, "Samantha", "Smith", false, "1/13/1984", "Tokyo" });
	_database->Include(cols, 6, new object[] { 6, "Andy", "Warhol", true, "9/14/1987", "New York" });
	_database->Include(cols, 7, new object[] { 7, "Carl", "Sagan", true, "4/1/1957", "Las Vegas" });
	*/
	boost::shared_ptr<Transaction> dataaccess = _database->Begin(true, true);

	dataaccess->Include(_columns, Encoder<ColumnID>::Encode(_columns[0]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[0]))("ID")("Int")("Int")(Encoder<BufferType>::Encode(BufferType::Identity))(Encoder<bool>::Encode(true)));
	dataaccess->Include(_columns, Encoder<ColumnID>::Encode(_columns[1]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[1]))("Given")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    dataaccess->Include(_columns, Encoder<ColumnID>::Encode(_columns[2]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[2]))("Surname")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    dataaccess->Include(_columns, Encoder<ColumnID>::Encode(_columns[3]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[3]))("Gender")("Bool")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    dataaccess->Include(_columns, Encoder<ColumnID>::Encode(_columns[4]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[4]))("BirthDate")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    dataaccess->Include(_columns, Encoder<ColumnID>::Encode(_columns[5]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[5]))("BirthPlace")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
}
    

