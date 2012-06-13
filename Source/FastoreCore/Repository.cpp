#include "Repository.h"
#include <sstream>

Repository::Repository(int columnID, const string& path) : _columnID(columnID), _path(path)
{
}

string Repository::GetLogFileName()
{
	ostringstream logFileName;
	logFileName << _path << "/" << _columnID << ".fslog";
	return logFileName.str();
}

string Repository::GetDataFileName(int number)
{
	ostringstream logFileName;
	logFileName << _path << "/" << _columnID << "_" << number << ".fsdata";
	return logFileName.str();
}

void Repository::Create()
{
	// Verify that there is no persistence to load from - shouldn't be if creating
}

void Repository::Load()
{
	// Update state to loading
	// Read header from each data file to determine which is newer
	ifstream dataFile(GetDataFileName(1));
	// ...
	// Read file into buffer, checking against CRC
	// If CRC checks out, take newer, otherwise take older

	//DataFileHeader header = ;
	//for (auto _log.GotoRevision(

	// Initialize the log file
	_log = auto_ptr<Log>(new Log(GetLogFileName()));

	// Read the head two pages of the log file
	// Take the newest non-torn page
	// Recover from data revision to last log entry
	// Set revision 
	
	// Update state to online
}

void Repository::Checkpoint()
{


}

