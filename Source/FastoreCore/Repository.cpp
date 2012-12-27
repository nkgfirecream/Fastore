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

using namespace boost::filesystem;

Repository::Repository(ColumnDef def) : _def(def), _revision(0)
{
	_status = RepositoryStatus::Offline;
	_buffer = createBuffer(_def);
	_status = RepositoryStatus::Online;
}

std::unique_ptr<IColumnBuffer> Repository::createBuffer(const ColumnDef& def)
{
	if (def.BufferType == BufferType_t::Identity)
	{
		if (def.RowIDType.Name != def.ValueType.Name)
			throw "Identity Buffers require rowType and ValueType to be the same";

		return std::unique_ptr<IColumnBuffer>(new IdentityBuffer(def.RowIDType));
	}
	else if(def.BufferType == BufferType_t::Unique)
	{
		//8 is the size of a pointer. If the size is less than 8, it's cheaper (from a memory point of view) to duplicate the value in the reverse index than it is to track pointers and update.
		if (def.ValueType.Size <= 8) 
		{
			return std::unique_ptr<IColumnBuffer>(new UniqueInlineBuffer(def.RowIDType, def.ValueType));
		}
		else
		{
			return std::unique_ptr<IColumnBuffer>(new UniqueBuffer(def.RowIDType, def.ValueType));
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
			return std::unique_ptr<IColumnBuffer>(new TreeBuffer(def.RowIDType, def.ValueType));		
		//}
	}
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
}

Statistic Repository::getStatistic()
{
	return _buffer->GetStatistic();
}
