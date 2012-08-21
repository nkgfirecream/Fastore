#include "TestSetup.h"

void TestSetup::createTableWithData()
{
	//connect
	ServiceAddress address;
	address.setName("localhost");
	address.setPort(8064);

	std::vector<ServiceAddress> addresses;
	addresses.push_back(address);

	auto _database = Client::Connect(addresses);

	_columns = list_of<ColumnID>(10000)(10001)(10002)(10003)(10004)(10005);

	//create schema
	_database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[0]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[0]))("ID")("Int")("Int")(Encoder<BufferType>::Encode(BufferType::Identity))(Encoder<bool>::Encode(true)));
	_database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[1]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[1]))("Given")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    _database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[2]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[2]))("Surname")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    _database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[3]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[3]))("Gender")("Bool")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    _database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[4]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[4]))("BirthDate")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    _database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[5]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[5]))("BirthPlace")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));

	//add data
	boost::shared_ptr<Transaction> dataaccess = _database->Begin(true, true);

	dataaccess->Include(_columns, Encoder<int>::Encode(0), list_of<std::string>(Encoder<int>::Encode(0))("Donny")("Osmond")(Encoder<bool>::Encode(true))("5/26/1980")("Antarctica"));
	dataaccess->Include(_columns, Encoder<int>::Encode(1), list_of<std::string>(Encoder<int>::Encode(1))("Marie")("Osmond")(Encoder<bool>::Encode(false))("4/20/1981")("Denver"));
    dataaccess->Include(_columns, Encoder<int>::Encode(2), list_of<std::string>(Encoder<int>::Encode(2))("Sarah")("Silverman")(Encoder<bool>::Encode(false))("11/10/1976")("Chicago"));
    dataaccess->Include(_columns, Encoder<int>::Encode(3), list_of<std::string>(Encoder<int>::Encode(3))("Bob")("Newhart")(Encoder<bool>::Encode(true))("12/2/1970")("Paris"));
    dataaccess->Include(_columns, Encoder<int>::Encode(4), list_of<std::string>(Encoder<int>::Encode(4))("Andy")("Warhol")(Encoder<bool>::Encode(true))("9/14/1987")("New York"));
    dataaccess->Include(_columns, Encoder<int>::Encode(5), list_of<std::string>(Encoder<int>::Encode(5))("Carl")("Sagan")(Encoder<bool>::Encode(true))("4/1/1957")("Tokyo"));
    dataaccess->Include(_columns, Encoder<int>::Encode(6), list_of<std::string>(Encoder<int>::Encode(6))("Marie")("Curie")(Encoder<bool>::Encode(false))("1/13/1984")("Las Vegas"));
}
    

