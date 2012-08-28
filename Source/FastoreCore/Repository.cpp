#include "Repository.h"
#include <fstream>
#include <sstream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "Column/UniqueBuffer.h"
#include "Column/TreeBuffer.h"
#include "Column/IdentityBuffer.h"

using namespace boost::filesystem;

Repository::Repository(int columnID, const string& path) : _columnID(columnID), _path(path)
{
	//set status to offline.
	_status = RepositoryStatus::Offline;
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

void Repository::create(ColumnDef def)
{
	if (def.ColumnID != _columnID)
		throw "Create definition columnId does not match assigned Id!";

	// Verify that there is no persistence to load from - shouldn't be if creating
	auto logpath = path(GetLogFileName());
	
	if (exists(logpath))
		throw "Existing log file!";

	for (int i = 0; i <= 1; i++)
	{
		auto datapath = path(GetDataFileName(i));
		if (exists(datapath))
			throw "Existing data file!";
	}
	
	//There's no previous data... continue with creation
	//Set our column definition
	_def = def;
	
	//Instatiante buffer
	if (_def.BufferType == BufferType_t::Identity)
	{
		if (_def.RowIDType.Name != _def.ValueType.Name)
			throw "Identity Buffers require rowType and ValueType to be the same";

		_buffer = new IdentityBuffer(_def.RowIDType);
	}
	else if(_def.BufferType == BufferType_t::Unique)
	{
		_buffer = new UniqueBuffer(_def.RowIDType, _def.ValueType);
	}
	else
	{
		_buffer = new TreeBuffer(_def.RowIDType, _def.ValueType);
	}


	// Initialize the log file
	//_log = auto_ptr<Log>(new Log(GetLogFileName()));

	//Set status to online
	_status = RepositoryStatus::Online;
}

void Repository::load()
{
	// Update state to loading
	_status = RepositoryStatus::Loading;

	// Read header from each data file to determine which is newer
	ifstream dataFile(GetDataFileName(1));
	// ...
	// Read file into buffer, checking against CRC
	// If CRC checks out, take newer, otherwise take older

	//DataFileHeader header = ;
	//for (auto _log.GotoRevision(

	// Initialize the log file
	//_log = auto_ptr<Log>(new Log(GetLogFileName()));

	// Read the head two pages of the log file
	// Take the newest non-torn page
	// Recover from data revision to last log entry
	// Set revision 
	
	// Update state to online
	_status = RepositoryStatus::Online;
}

//TODO: we've discussed letting the service handle the checkpointing.
void Repository::checkpoint()
{
	// Set state to checkpointing
	_status = RepositoryStatus::Checkpointing;

	// Pick oldest datafile
	ofstream dataFile(GetDataFileName(1));
	// ...
	// Write buffer to file

	// Truncate log

	// Update state to online
	_status = RepositoryStatus::Online;
}

Answer Repository::query(const fastore::communication::Query& query)
{
	Answer answer;	
	if (query.ranges.size() > 0)
	{
		std::vector<RangeResult> results;
		for (int i = 0; i < query.ranges.size(); i++)
		{
			
			auto range = query.ranges[i];
			RangeResult result = _buffer->GetRows(range);
			results.push_back(result);
		}

		answer.__set_rangeValues(results);
	}

	if (query.rowIDs.size() > 0)
	{
		auto values = _buffer->GetValues(query.rowIDs);
		answer.__set_rowIDValues(values);
	}

	return answer;
}

void Repository::apply(const Revision& revision, const ColumnWrites& writes)
{
	_buffer->Apply(writes);
	_revision = revision;

	WriteToLog(revision, writes);
}

void Repository::WriteToLog(const Revision& revision, const ColumnWrites& writes)
{
	//Buffer log write? Batched log write?
}

Statistic Repository::getStatistic()
{
	return _buffer->GetStatistic();
}

void Repository::drop()
{
	_status = RepositoryStatus::Offline;
}

