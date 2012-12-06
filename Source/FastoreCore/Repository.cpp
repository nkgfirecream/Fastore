#include "Repository.h"
#include <fstream>
#include <sstream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "../FastoreCommon/Buffer/UniqueBuffer.h"
#include "../FastoreCommon/Buffer/UniqueInlineBuffer.h"
#include "../FastoreCommon/Buffer/TreeBuffer.h"
#include "../FastoreCommon/Buffer/TreeInlineBuffer.h"
#include "../FastoreCommon/Buffer/IdentityBuffer.h"
#include "TFastoreFileTransport.h"
#include <thrift/protocol/TBinaryProtocol.h>
//#include "Column/MultiBimapBuffer.h"

using namespace boost::filesystem;

Repository::Repository(ColumnID columnID, const string& path) : _columnID(columnID), _path(path)
{	
	load();
}

Repository::Repository(ColumnDef def, const string& path) : _def(def), _path(path), _columnID(def.ColumnID)
{
	create();
}

void Repository::checkExists()
{
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

void Repository::create()
{
	_status = RepositoryStatus::Offline;
	//checkExists();
	initializeBuffer();
	//initializeLog();
	_status = RepositoryStatus::Online;
}

void Repository::initializeBuffer()
{
	if (_def.BufferType == BufferType_t::Identity)
	{
		if (_def.RowIDType.Name != _def.ValueType.Name)
			throw "Identity Buffers require rowType and ValueType to be the same";

		_buffer = std::unique_ptr<IColumnBuffer>(new IdentityBuffer(_def.RowIDType));
	}
	else if(_def.BufferType == BufferType_t::Unique)
	{
		//8 is the size of a pointer. If the size is less than 8, it's cheaper (from a memory point of view) to duplicate the value in the reverse index than it is to track pointers and update.
		if (_def.ValueType.Size <= 8) 
		{
			_buffer = std::unique_ptr<IColumnBuffer>(new UniqueInlineBuffer(_def.RowIDType, _def.ValueType));
		}
		else
		{
			_buffer = std::unique_ptr<IColumnBuffer>(new UniqueBuffer(_def.RowIDType, _def.ValueType));
		}
	}
	else
	{
		//if (_def.ValueType.Size <= 8)
		//{
		//	_buffer = std::unique_ptr<IColumnBuffer>(new TreeInlineBuffer(_def.RowIDType, _def.ValueType));		
		//}
		//else
		//{
			_buffer = std::unique_ptr<IColumnBuffer>(new TreeBuffer(_def.RowIDType, _def.ValueType));		
		//}
	}
}

void Repository::load()
{
	//TODO: refactor into set status? Should repos load in a separate thread
	//so the worker can keep doing its thing?
	// Update state to loading
	_status = RepositoryStatus::Loading;

	// Read header from each data file to determine which is newer
	// and complete (via crc check)
	auto datapath = path(GetDataFileName(0));
	if (!exists(datapath))
		throw "Can't load repo. File not found!";
	
	//Open file
	boost::shared_ptr<apache::thrift::transport::TFastoreFileTransport> transport( new apache::thrift::transport::TFastoreFileTransport(datapath.string(), true));	
	boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol> protocol(new apache::thrift::protocol::TBinaryProtocol(transport));

	transport->open();

	//Read column definition
	ColumnDef def;

	int64_t columnId;
	protocol->readI64(columnId);
	def.ColumnID = columnId;

	int buffer;
	protocol->readI32(buffer);
	def.BufferType = (BufferType_t)buffer;

	string name;
	protocol->readString(name);
	def.Name = name;

	bool required;
	protocol->readBool(required);
	def.Required = required;

	string rowType;
	protocol->readString(rowType);
	def.RowIDType = standardtypes::GetTypeFromName(rowType);

	string valueType;
	protocol->readString(valueType);
	def.ValueType = standardtypes::GetTypeFromName(valueType);


	//Now have a definition, so initalize  the buffer
	_def = def;
	initializeBuffer();

	//Read rest of file into buffer	
	while(protocol->getTransport()->peek())
	{
		ColumnWrites writes;
		vector<Cell> includes;
		ValueRows vr;
		vr.read(protocol.get());
		for (size_t j = 0; j < vr.rowIDs.size(); j++)
		{
			Cell inc;
			inc.__set_value(vr.value);
			inc.__set_rowID(vr.rowIDs[j]);
			includes.push_back(inc);
		}

		writes.__set_includes(includes);
		_buffer->Apply(writes);
	}	

	transport->close();

	//initialize log if not present
	//if (no log)
	//intialize log()
	//else
	//read log()

	// Read the head two pages of the log file
	// Take the newest non-torn page
	// Recover from data revision to last log entry
	// Set revision 
	
	// Update state to online
	_status = RepositoryStatus::Online;
}

Answer Repository::query(const fastore::communication::Query& query)
{
	Answer answer;	
	if (query.ranges.size() > 0)
	{
		std::vector<RangeResult> results;
		for (size_t i = 0; i < query.ranges.size(); i++)
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

