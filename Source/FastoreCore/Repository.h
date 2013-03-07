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
	RepositoryStatus::type _status;
	ColumnDef _def;
	std::unique_ptr<IColumnBuffer> _buffer;
	Revision _revision;

	static std::unique_ptr<IColumnBuffer> createBuffer(const ColumnDef& def);

public:

	//Create repo
	Repository(ColumnDef def);

	ColumnID getColumnID() { return _def.ColumnID; }

	RepositoryStatus::type getStatus() { return _status; }
	ColumnDef getDef() { return _def; }
	Revision getRevision() { return _revision; }
	Statistic getStatistic();

	void setRevision(Revision revision) { _revision = revision; } 

	Answer query(const Query& query);
	void apply(const Revision& revision, const ColumnWrites& writes);

};