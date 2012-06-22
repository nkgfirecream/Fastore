#pragma once
#include <string>
#include <exception>
#include <memory>
#include "Schema\column.h"
#include "Logging/Log.h"
#include "Column\IColumnBuffer.h"
#include "../FastoreCommunication/Comm_types.h"

using namespace ::fastore::communication;
using namespace std;

class Repository
{
private:
	int _columnID;
	string _path;
	RepositoryStatus _status;
	ColumnDef _def;
	boost::shared_ptr<Log> _log;
	IColumnBuffer* _buffer;
	Revision _revision;

	string GetLogFileName();
	string GetDataFileName(int number);
	
	void WriteToLog(const Revision& revision, const ColumnWrites& writes);

public:
	Repository(int columnID, const string& path);

	int getColumnID() { return _columnID; }
	string getPath() { return _path; }
	RepositoryStatus getStatus() { return _status; }
	ColumnDef getDef() { return _def; }
	Revision getRevision() { return _revision; }

	//Attempts to load from disk
	void load();

	//Unload instead? Saves to disk and releases memory
	void shutdown();

	//Initializes the repo matching the definition
	void create(ColumnDef def);

	//Frees memory, erases physical files
	void destroy();

	//Saves to disk
	void checkpoint();

	//Query column
	Answer query(const Query& query);

	//Apply writes to column
	void apply(const Revision& revision, const ColumnWrites& writes);

	//Get statistics about  column
	Statistic getStatistic();
};