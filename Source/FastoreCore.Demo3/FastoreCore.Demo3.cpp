// FastoreCore.Demo3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\FastoreClient\ServiceAddress.h"
#include "..\FastoreClient\Database.h"
#include "..\FastoreClient\Client.h"
#include "..\FastoreClient\Dictionary.h"
#include "..\FastoreClient\Encoder.h"
#include <vector>
#include <boost\assign\list_of.hpp>

using namespace boost::assign;
using namespace fastore::client;

int _tmain(int argc, _TCHAR* argv[])
{
	ServiceAddress address;
	address.setName("localhost");
	address.setPort(ServiceAddress::DefaultPort);

	std::vector<ServiceAddress> addresses;
	addresses.push_back(address);

	std::vector<ColumnID> _columns = list_of<ColumnID>(10000)(10001)(10002)(10003)(10004)(10005);

	auto _database = Client::Connect(addresses);


	//Create schema
	_database.Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[0]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[0]))("ID")("Int")("Int")(Encoder<BufferType>::Encode(BufferType::Identity)));
	_database.Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[1]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[1]))("Given")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi)));
    _database.Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[2]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[2]))("Surname")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi)));
    _database.Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[3]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[3]))("Gender")("Bool")("Int")(Encoder<BufferType>::Encode(BufferType::Multi)));
    _database.Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[4]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[4]))("BirthDate")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi)));
    _database.Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[5]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[5]))("BirthPlace")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi)));

	Range podIdRange;
	podIdRange.Ascending = true;
	podIdRange.ColumnID = Dictionary::PodID;

	std::vector<ColumnID> podidv;
	podidv.push_back(Dictionary::PodID);

	auto podIds = _database.GetRange(podidv, podIdRange, 500);

	for (int i = 0; i < _columns.size(); i++)
	{
		_database.Include(Dictionary::PodColumnColumns, Encoder<int>::Encode(i), list_of<std::string>(podIds.getData()[i % podIds.getData().getCount()].Values[0])(Encoder<ColumnID>::Encode(_columns[i])));
	}

}

