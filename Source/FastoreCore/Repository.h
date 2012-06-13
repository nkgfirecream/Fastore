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

	string GetLogFileName();
	string GetDataFileName(int number);

public:
	Repository(int columnID, const string& path);

	int getColumnID() { return _columnID; }
	string getPath() { return _path; }
	RepositoryStatus getStatus() { return _status; }
	ColumnDef getDef() { return _def; }

	void Load();
	void Create();
	void Checkpoint();
};