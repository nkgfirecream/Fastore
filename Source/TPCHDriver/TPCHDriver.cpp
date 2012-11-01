// TPCHDriver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <vector>

#include "..\FastoreProvider\fastore.h"

const static int NUM_QUERIES = 22;
const static int NUM_SETS = 41;
const static int NUM_STREAMS = 1; //This may be variable depending how many client we have processing.
const static double SCALE_FACTOR = .001; //This depends on the generated data, and may change as an argument or if we generate data via the driver.
const char* const SCRIPT_PATH = "..\\SCRIPTS\\";
const char* const TPCH_DEF = "TPCH";
const static int TRANSACTION_SIZE = 100;
const std::string TABLES[] =
{
	"CUSTOMER",
	"LINEITEM",
	"NATION",
	"ORDERS",
	"PART",
	"PARTSUPP",
	"REGION",
	"SUPPLIER"
};

//41 sets of 22 queries in various orders.
//set 0 is power test
//sets 1 - 40 are the throughput tests
const int testSets[NUM_SETS][NUM_QUERIES] =
{
	{14, 2,	9,	20,	6,	17,	18,	8,	21,	13,	3,	22,	16,	4,	11,	15,	1,	10,	19,	5,	7,	12},
	{21,3,	18,	5,	11,	7,	6,	20,	17,	12,	16,	15,	13,	10,	2,	8,	14,	19,	9,	22,	1,	4},
	{6,	17,	14,	16,	19,	10,	9,	2,	15,	8,	5,	22,	12,	7,	13,	18,	1,	4,	20,	3,	11,	21},
	{8,	5,	4,	6,	17,	7,	1,	18,	22,	14,	9,	10,	15,	11,	20,	2,	21,	19,	13,	16,	12,	3},
	{5,	21,	14,	19,	15,	17,	12,	6,	4,	9,	8,	16,	11,	2,	10,	18,	1,	13,	7,	22,	3,	20},
	{21,15,	4,	6,	7,	16,	19,	18,	14,	22,	11,	13,	3,	1,	2,	5,	8,	20,	12,	17,	10,	9},
	{10,3,	15,	13,	6,	8,	9,	7,	4,	11,	22,	18,	12,	1,	5,	16,	2,	14,	19,	20,	17,	21},
	{18,8,	20,	21,	2,	4,	22,	17,	1,	11,	9,	19,	3,	13,	5,	7,	10,	16,	6,	14,	15,	12},
	{19,1,	15,	17,	5,	8,	9,	12,	14,	7,	4,	3,	20,	16,	6,	22,	10,	13,	2,	21,	18,	11},
	{8,	13,	2,	20,	17,	3,	6,	21,	18,	11,	19,	10,	15,	4,	22,	1,	7,	12,	9,	14,	5,	16},
	{6,	15,	18,	17,	12,	1,	7,	2,	22,	13,	21,	10,	14,	9,	3,	16,	20,	19,	11,	4,	8,	5},
	{15,14,	18,	17,	10,	20,	16,	11,	1,	8,	4,	22,	5,	12,	3,	9,	21,	2,	13,	6,	19,	7},
	{1,	7,	16,	17,	18,	22,	12,	6,	8,	9,	11,	4,	2,	5,	20,	21,	13,	10,	19,	3,	14,	15},
	{21,17,	7,	3,	1,	10,	12,	22,	9,	16,	6,	11,	2,	4,	5,	14,	8,	20,	13,	18,	15,	19},
	{2,	9,	5,	4,	18,	1,	20,	15,	16,	17,	7,	21,	13,	14,	19,	8,	22,	11,	10,	3,	12,	6},
	{16,9,	17,	8,	14,	11,	10,	12,	6,	21,	7,	3,	15,	5,	22,	20,	1,	13,	19,	2,	4,	18},
	{1,	3,	6,	5,	2,	16,	14,	22,	17,	20,	4,	9,	10,	11,	15,	8,	12,	19,	18,	13,	7,	21},
	{3,	16,	5,	11,	21,	9,	2,	15,	10,	18,	17,	7,	8,	19,	14,	13,	1,	4,	22,	20,	6,	12},
	{14,4,	13,	5,	21,	11,	8,	6,	3,	17,	2,	20,	1,	19,	10,	9,	12,	18,	15,	7,	22,	16},
	{4,	12,	22,	14,	5,	15,	16,	2,	8,	10,	17,	9,	21,	7,	3,	6,	13,	18,	11,	20,	19,	1},
	{16,15,	14,	13,	4,	22,	18,	19,	7,	1,	12,	17,	5,	10,	20,	3,	9,	21,	11,	2,	6,	8},
	{20,14,	21,	12,	15,	17,	4,	19,	13,	10,	11,	1,	16,	5,	18,	7,	8,	22,	9,	6,	3,	2},
	{16,14,	13,	2,	21,	10,	11,	4,	1,	22,	18,	12,	19,	5,	7,	8,	6,	3,	15,	20,	9,	17},
	{18,15,	9,	14,	12,	2,	8,	11,	22,	21,	16,	1,	6,	17,	5,	10,	19,	4,	20,	13,	3,	7},
	{7,	3,	10,	14,	13,	21,	18,	6,	20,	4,	9,	8,	22,	15,	2,	1,	5,	12,	19,	17,	11,	16},
	{18,1,	13,	7,	16,	10,	14,	2,	19,	5,	21,	11,	22,	15,	8,	17,	20,	3,	4,	12,	6,	9},
	{13,2,	22,	5,	11,	21,	20,	14,	7,	10,	4,	9,	19,	18,	6,	3,	1,	8,	15,	12,	17,	16},
	{14,17,	21,	8,	2,	9,	6,	4,	5,	13,	22,	7,	15,	3,	1,	18,	16,	11,	10,	12,	20,	19},
	{10,22,	1,	12,	13,	18,	21,	20,	2,	14,	16,	7,	15,	3,	4,	17,	5,	19,	6,	8,	9,	11},
	{10,8,	9,	18,	12,	6,	1,	5,	20,	11,	17,	22,	16,	3,	13,	2,	15,	21,	14,	19,	7,	4},
	{7,	17,	22,	5,	3,	10,	13,	18,	9,	1,	14,	15,	21,	19,	16,	12,	8,	6,	11,	20,	4,	2},
	{2,	9,	21,	3,	4,	7,	1,	11,	16,	5,	20,	19,	18,	8,	17,	13,	10,	12,	15,	6,	14,	22},
	{15,12,	8,	4,	22,	13,	16,	17,	18,	3,	7,	5,	6,	1,	9,	11,	21,	10,	14,	20,	19,	2},
	{15,16,	2,	11,	17,	7,	5,	14,	20,	4,	21,	3,	10,	9,	12,	8,	13,	6,	18,	19,	22,	1},
	{1,	13,	11,	3,	4,	21,	6,	14,	15,	22,	18,	9,	7,	5,	10,	20,	12,	16,	17,	8,	19,	2},
	{14,17,	22,	20,	8,	16,	5,	10,	1,	13,	2,	21,	12,	9,	4,	18,	3,	7,	6,	19,	15,	11},
	{9,	17,	7,	4,	5,	13,	21,	18,	11,	3,	22,	1,	6,	16,	20,	14,	15,	10,	8,	2,	12,	19},
	{13,14,	5,	22,	19,	11,	9,	6,	18,	15,	8,	10,	7,	4,	17,	16,	3,	1,	12,	2,	21,	20},
	{20,5,	4,	14,	11,	1,	6,	16,	8,	22,	7,	3,	2,	12,	21,	19,	17,	13,	10,	15,	18,	9},
	{3,	7,	14,	15,	6,	5,	21,	20,	18,	10,	4,	16,	19,	1,	13,	9,	8,	17,	11,	12,	22,	2},
	{13,15,	17,	1,	22,	11,	3,	4,	7,	20,	14,	21,	9,	8,	2,	18,	16,	6,	10,	12,	5,	19}
};

std::string queries[NUM_QUERIES];

ConnectionHandle _connection;

std::string ReadAll(std::string filename)
{
	std::ifstream f(filename);

	if (!f.is_open())
		throw "Error opening filestream!";

	std::string result;

	f.seekg(0, std::ios::end);
	result.reserve(f.tellg());
	f.seekg(0, std::ios::beg);

	result.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

	return result;
}

std::string LoadQuery(int i)
{
	std::stringstream path;
	path << SCRIPT_PATH << i << ".out";
	return ReadAll(path.str());
}

bool DetectSchema()
{
	std::cout << "Detecting schema...\n";
	auto result = fastoreExecute(_connection, ("select * from sqlite_master where name = '" + TABLES[0] + "';").c_str());
	return !result.eof;
}

void InsertRecord(std::string& table, std::vector<std::string>& record)
{
	std::stringstream ss;

	ss << "insert into " << table << " values(";

	bool first = true;
	for (int i = 0; i < record.size(); i++)
	{
		if(!first)
			ss << ", ";

		first = false;

		ss << "'" << record[i] << "'";
	}
	ss << ")";

	auto result = fastoreExecute(_connection, ss.str().c_str());
}

void ImportTable(std::string tablename)
{
	std::cout << "Importing table: " << tablename << "...\n";

	std::stringstream path;
	path << SCRIPT_PATH << tablename << ".tbl";

	std::ifstream infile(path.str());

	auto result = fastoreExecute(_connection, "begin");
	int num = 0;
	while (infile)
	{
		std::string s;
		if (!getline( infile, s )) break;

		std::istringstream ss( s );
		std::vector<std::string> record;

		while (ss)
		{
			std::string s;
			if (!getline( ss, s, '|' )) break;
				record.push_back( s );
		}

		InsertRecord(tablename, record);
		num++;

		if (num % TRANSACTION_SIZE == 0)
		{
			std::cout << num << std::endl;
			result = fastoreExecute(_connection, "commit");
			result = fastoreExecute(_connection, "begin");
		}
	}

	result = fastoreExecute(_connection, "commit");

	std::cout << "Inserted " << num << " rows\n";
}

void ImportTables()
{
	std::cout << "Importing TPCH data...\n";
	for (int i = 0; i < (sizeof(TABLES) / sizeof(std::string)); ++i)
	{
		ImportTable(TABLES[i]);
	}
}

void ConnectToFastore()
{
	std::cout << "Connecting to Fastore server...\n";
	FastoreAddress addresses[1] = { { "localhost", 8765} };
	
	auto result = fastoreConnect(1, addresses);

	if (result.result == FASTORE_OK)
	{
		_connection = result.connection;
	}
	else
	{
		throw "Error connecting to Fastore server!";
	}
}

void CreateTableDefs()
{
	std::cout << "Creating TPCH schema...\n";
	std::stringstream path;
	path << SCRIPT_PATH << TPCH_DEF << ".sql";

	std::string sql = ReadAll(path.str());
	std::stringstream ss(sql);
	std::vector<std::string> statements;

	while (ss)
	{
		std::string s;
		if (!getline( ss, s, ';' )) break;
		statements.push_back(s);
	}

	for (int i = 0; i < statements.size(); ++i)
	{
		auto result = fastoreExecute(_connection, statements[i].c_str());
		if (result.result != FASTORE_OK)
			throw "Error creating TPCH schema!";
	}	
}

void LoadQueries()
{
	std::cout << "Loading queries...\n";
	for (int i = 0; i < NUM_QUERIES; ++i)
	{
		queries[i] = LoadQuery(i + 1);
	}
}

long long _interval;
void ThroughputTest()
{
	std::cout << "Running throughput test...\n";
	long long start = clock();

	for (int set = 1; set <= NUM_STREAMS/*NUM_SETS -- one set per stream*/; ++set)
	{
		std::cout << "Executing set: " << set << std::endl;
#if 0
		for (int query = 0; query <= NUM_QUERIES; ++query)
#else
		for (int query = 2; query <= 2 /*NUM_QUERIES*/; ++query)
#endif
		{
			std::cout << "Executing query: " << query << " (" << (testSets[set][query] + 1) << ".OUT)" << std::endl;
			auto result = fastorePrepare(_connection, (queries[testSets[set][query]]).c_str());
			if (result.result != FASTORE_OK)
			{
				throw "Error preparing query!";
			}			

			NextResult data;
			do
			{
				data = fastoreNext(result.statement);
				if (data.result != FASTORE_OK)
				{
					throw "Error executing query!";
				}
			} 
			while(!data.eof);

		}
	}

	long long end = clock();
	_interval = end - start;
	std::cout << "Seconds to execute throughput test: " << (_interval / 1000) << std::endl;
}

double _throughput;
void CalculateMetrics()
{
	std::cout << "Calculating metrics...\n";
	_throughput = (NUM_STREAMS * NUM_QUERIES * 3600) / (_interval / (double)1000) * SCALE_FACTOR;
}

void DisplayMetrics()
{
	std::cout.precision(1);
	std::cout << "Throughput performance: " << _throughput << " Queries per hour @ Scale-Factor\n";
}

int _tmain(int argc, _TCHAR* argv[])
{
	//TODO: Future version of the driver:
	// Take connection strings as arguments
	// Take a scale factor as an argument
	// Generate Data according to the scale factor (by executing dbgen and qgen)
	// Execute multiple clients (if they exist on different machines)
	// Optionally Check correctness of queries 
	// Execute power function correctly (execute refresh functions -- see TPCH doc Power definition)
		
	LoadQueries();	
	ConnectToFastore();	

	if (!DetectSchema())
	{
		std::cout << "Schema not found. Creating data...\n";
		CreateTableDefs();
		ImportTables();
	}
	else
	{
		std::cout << "Schema detected. Skipping data creation...\n";
	}

	std::cout << "Setup complete. Press any key to begin tests.\n";
	std::getchar();

	//Run and time power test
	std::cout << "TODO: Power Test\n";
	// Skip for now. We need to add refresh functions. Composite metric is sqrt(power * throughput), so just taking
	// throughput is a *rough* approximation (and probably on the high side!)

	ThroughputTest();	
	CalculateMetrics();
	DisplayMetrics();

	std::cout << "Press any key to close.\n";
	std::getchar();
	return 0;
}



