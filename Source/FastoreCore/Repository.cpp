#include "Repository.h"
#include <sstream>

Repository::Repository(int columnID, const string& path, const ColumnDef& def) : _columnID(columnID), _path(path), _def(def)
{
}

string Repository::GetLogFileName()
{
	ostringstream logFileName;
	logFileName << _path << "\\" << _columnID << ".fslog";
	return logFileName.str();
}

string Repository::GetDataFileName()
{
	ostringstream logFileName;
	logFileName << _path << "\\" << _columnID << ".fsdata";
	return logFileName.str();
}

void Repository::Create()
{
	// Verify that there is no persistence to load from - shouldn't be if creating
}

void Repository::Load()
{
	// Initialize the log file
	_log = auto_ptr<Log>(new Log(GetLogFileName()));

	ifstream dataFile(GetDataFileName());
	//DataFileHeader header = ;
	//for (auto _log.GotoRevision(

	//* Read topology columns into memory; play log files for the same
	//* Apply any host address overrides
	//* Determine host ID if by reversing host address - if fails throw with report of current host name
	//* Mode: Initializing - Notify peers (hosts that share redundancy), storing which peers are reachable in Grid Health memory structures.
	//* Initialize objects for this host (simultaneously unless it hurts)
	//	* Column Buffer - Read data dump into memory; play log file
	//	* Lock Manager - sync w/ peer(s)
	//	* Transactor - sync w/ peer(s)
	//* Mode: Online - Notify peers
}

