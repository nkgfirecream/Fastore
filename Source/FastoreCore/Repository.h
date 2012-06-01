#pragma once

#include <string>
#include "Schema\column.h"
#include <exception>
#include <memory>
#include "Logging/Log.h"

using namespace std;

enum RepositoryStatus
{
    Loading = 1,
    Unloading = 2,
    Online = 3,
    Checkpointing = 4,
	Offline = 5
};

class Repository
{
private:
	int _columnID;
	string _path;
	RepositoryStatus _status;
	ColumnDef _def;
	auto_ptr<Log> _log;

	string GetLogFileName();
	string GetDataFileName();

public:
	Repository(int columnID, const string& path, const ColumnDef& def);

	int getColumnID() { return _columnID; }
	string getPath() { return _path; }
	RepositoryStatus getStatus() { return _status; }
	ColumnDef getDef() { return _def; }

	void Load();
	void Create();
	void Checkpoint();
};