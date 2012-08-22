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
#include <iostream>
#include <fstream>

using namespace boost::assign;
using namespace fastore::client;


void LineToRecord(const char* line, std::vector<std::string>& record)
{
	int cell = 0;
	int i = 0;
	std::vector<char> builder;
	while (i < 2048)
	{
		auto ch = line[i];
		if (ch == '\"')
		{
			i++;
			while (i < 2048 && (ch = line[i]) != '\"')
			{
				if (ch == '\\')
				{
					i++;
					if (i >= 2048)
						throw "Invalid escape sequence";
					ch = line[i];
					switch (ch)
					{
						case 'n' : builder.push_back('\n'); break;
						default: builder.push_back(ch); break;
					}
				}
				else if (ch == '\0')
				{
					break;
				}
				else
				{
					builder.push_back(ch);
				}
				i++;
			}
			if (ch == '\0')
				break;
			if (ch != '\"')
				throw "Unterminated quote.";
		}
		else if (ch == ',' || ch == '\0')
		{
			record[cell] = std::string(builder.begin(), builder.end());
			cell++;
			builder.clear();
		}

		if (ch == '\0')
		{
			//End of line
			break;
		}
		i++;
	}
}

void InsertRecord(std::vector<std::string>& record, std::vector<ColumnID>& columns, IDataAccess* dataaccess)
{
	std::vector<std::string> encoded(8, std::string());

	for (int i = 0; i < columns.size() && i < record.size(); i++)
	{
		switch (i)
		{
			case 0:
				encoded[i] = Encoder<int>::Encode(std::stoi(record[i])); break;
			case 3:
				encoded[i] = record[i] == "0" ? Encoder<bool>::Encode(false) : Encoder<bool>::Encode(true); break;
			default:
				encoded[i] = record[i]; break;
		}

	}
	dataaccess->Include(columns, encoded[0], encoded);
}

int _tmain(int argc, _TCHAR* argv[])
{
	ServiceAddress address;
	address.Name = "localhost";
	address.Port = ServiceAddress::DefaultPort;

	std::vector<ServiceAddress> addresses;
	addresses.push_back(address);

	std::vector<ColumnID> _columns = list_of<ColumnID>(10000)(10001)(10002)(10003)(10004)(10005);

	auto _database = Client::Connect(addresses);


	//Create schema
	_database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[0]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[0]))("ID")("Int")("Int")(Encoder<BufferType>::Encode(BufferType::Identity))(Encoder<bool>::Encode(true)));
	_database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[1]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[1]))("Given")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    _database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[2]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[2]))("Surname")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    _database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[3]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[3]))("Gender")("Bool")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    _database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[4]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[4]))("BirthDate")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));
    _database->Include(Dictionary::ColumnColumns, Encoder<ColumnID>::Encode(_columns[5]), list_of<std::string>(Encoder<ColumnID>::Encode(_columns[5]))("BirthPlace")("String")("Int")(Encoder<BufferType>::Encode(BufferType::Multi))(Encoder<bool>::Encode(true)));

	Range podIdRange;
	podIdRange.Ascending = true;
	podIdRange.ColumnID = Dictionary::PodID;

	std::vector<ColumnID> podidv;
	podidv.push_back(Dictionary::PodID);

	auto podIds = _database->GetRange(podidv, podIdRange, 500);

	for (int i = 0; i < _columns.size(); i++)
	{
		_database->Include(Dictionary::PodColumnColumns, Encoder<int>::Encode(i), list_of<std::string>(podIds.getData()[i % podIds.getData().size()].Values[0])(Encoder<ColumnID>::Encode(_columns[i])));
	}


	std::string filename = "e:\\ancestry\\owt\\owt.csv";

	std::ifstream file;
	file.open(filename);


	int count = 0;
	clock_t lastMilliseconds = 0;
	clock_t stopwatchStart = 0;
	clock_t stopwatchEnd = 0;

	char line[2048];

	std::vector<std::string> record(8);

	stopwatchStart = clock();

	//boost::shared_ptr<IDataAccess> dataaccess = _database;
	boost::shared_ptr<Transaction> dataaccess = _database->Begin(true, true);
	std::future<void> committask;

	int batchSize = 5000;
	while (!file.eof() && count < 10000000)
	{
		file.getline(line, 2048);

		count++;

		LineToRecord(line, record);
		InsertRecord(record, _columns, dataaccess.get());
		

		if (count % batchSize == 0)
		{
			if (committask.valid())
				committask.wait();

			committask = std::async
			(
				std::launch::async,
				[dataaccess]()
				{
					dataaccess->Commit();
				}
			);

			clock_t elapsed = clock() - lastMilliseconds;
			if (elapsed == 0)
				elapsed = 1;
			lastMilliseconds = clock();
			std::cout << "Loaded: " << count << " Last Rate: " << 1000 / ((double)elapsed / batchSize) << " rows/sec\n";
			dataaccess = _database->Begin(true, true);
		}

	}
}



