#pragma once
#include <string>
#include <exception>
#include <memory>
#include <Buffer/ColumnDef.h>
//#include "Logging/Log.h"
#include <Buffer/IColumnBuffer.h>
#include <Communication/Comm_types.h>

using namespace ::fastore::communication;
using namespace std;

class Repository
{
private:
	ColumnID _columnID;
	RepositoryStatus::type _status;
	ColumnDef _def;
	//boost::shared_ptr<Log> _log;
	std::unique_ptr<IColumnBuffer> _buffer;
	Revision _revision;

	//string GetLogFileName();
	//string GetDataFileName(int number);
	
	//Attempts to load from disk
	//void load();

	//Attempts to create a new repo
	void create();

	//Makes sure no files are present before create a new repo
	//void checkExists();

	//Initializes a buffer repo matching the definition
	void initializeBuffer();
	
	//void WriteToLog(const Revision& revision, const ColumnWrites& writes);

public:
	//Load repo
	//Repository(ColumnID columnID, const string& path);

	//Create repo
	Repository(ColumnDef def);

	ColumnID getColumnID() { return _columnID; }
	//string getPath() { return _path; }
	RepositoryStatus::type getStatus() { return _status; }
	ColumnDef getDef() { return _def; }
	Revision getRevision() { return _revision; }

	//Unload instead? Saves to disk and releases memory
	void shutdown();

	//Frees buffer, erases physical files
	void drop();

	//Query column
	Answer query(const Query& query);

	//Apply writes to column
	void apply(const Revision& revision, const ColumnWrites& writes);

	//Get statistics about  column
	Statistic getStatistic();
};