#include "Repository.h"
#include <fstream>
#include <sstream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <Buffer/BufferFactory.h>
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
	return BufferFactory::CreateBuffer(def.ValueTypeName, def.RowIDTypeName, def.BufferType);
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
