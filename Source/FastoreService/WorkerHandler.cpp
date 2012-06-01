#include "WorkerHandler.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <utility>

WorkerHandler::WorkerHandler(const ServiceConfig& config) : _host(config.coreConfig)
{
}

Revision WorkerHandler::prepare(const TransactionID& transactionID, const Writes& writes, const Reads& reads) 
{
	// Your implementation goes here
	printf("Prepare\n");

	return 0;
}

void WorkerHandler::apply(TransactionID& _return, const TransactionID& transactionID, const Writes& writes) 
{
	bool syncSchema = false;

	auto start = writes.begin();

	while(start != writes.end())
	{
		fastore::ColumnID id = (*start).first;

		if (id == 0)
			syncSchema = true;

		fastore::ColumnWrites writes = (*start).second;

		PointerDefPair pdp = _host.GetColumn(id);

		auto exStart = writes.excludes.begin();
		while (exStart != writes.excludes.end())
		{
			//TODO: Fix Leaks
			void* rowIdp = pdp.second.RowIDType.Allocate();			
			pdp.second.RowIDType.Decode((*exStart).rowID, rowIdp);
			pdp.first->Exclude(rowIdp);

			delete rowIdp;
			exStart++;
		}

		auto inStart = writes.includes.begin();
		while (inStart != writes.includes.end())
		{
			void* rowIdp = pdp.second.RowIDType.Allocate();
			void* valuep = pdp.second.ValueType.Allocate();

			pdp.second.RowIDType.Decode((*inStart).rowID, rowIdp);
			pdp.second.ValueType.Decode((*inStart).value, valuep);

			pdp.first->Include(valuep, rowIdp);

			delete valuep, rowIdp;

			inStart++;
		}

		start++;
	}

	if (syncSchema)
		_host.SyncToSchema();
}

void WorkerHandler::commit(const TransactionID& transactionID) {
// Your implementation goes here
printf("Commit\n");
}

void WorkerHandler::rollback(const TransactionID& transactionID) {
// Your implementation goes here
printf("Rollback\n");
}

void WorkerHandler::flush(const TransactionID& transactionID) {
// Your implementation goes here
printf("Flush\n");
}

bool WorkerHandler::doesConflict(const Reads& reads, const Revision source, const Revision target) {
// Your implementation goes here
printf("DoesConflict\n");

return false;
}

void WorkerHandler::update(TransactionID& _return, const TransactionID& transactionID, const Writes& writes, const Reads& reads) {
// Your implementation goes here
printf("Update\n");
}

void WorkerHandler::transgrade(Reads& _return, const Reads& reads, const Revision source, const Revision target) {
// Your implementation goes here
printf("Transgrade\n");
}


void WorkerHandler::query(ReadResults& _return, const Queries& queries)
{
	auto start = queries.begin();

	while(start != queries.end())
	{
		fastore::ColumnID id = (*start).first;
		fastore::Query query = (*start).second;

		PointerDefPair pdp = _host.GetColumn(id);

		fastore::ReadResult res;

		if (query.ranges.size() > 0)
		{
			for (int i = 0; i < query.ranges.size(); i++)
			{
				auto range = query.ranges[i];

				Optional<fs::RangeBound> starto;
				Optional<fs::RangeBound> endo;

				void* ostartp = NULL;
				void* oendp = NULL;

				if (range.__isset.first)
				{
					fs::RangeBound rb;
					rb.Inclusive = range.first.inclusive;
					rb.Value = pdp.second.ValueType.Allocate();
					pdp.second.ValueType.Decode(range.first.value, rb.Value);
					starto = Optional<fs::RangeBound>(rb);
				}

				if (range.__isset.last)
				{
					fs::RangeBound rb;
					rb.Inclusive = range.last.inclusive;
					rb.Value = pdp.second.ValueType.Allocate();
					pdp.second.ValueType.Decode(range.last.value, rb.Value);
					endo = Optional<fs::RangeBound>(rb);
				}		

				Optional<void*> startId;
				if (range.__isset.rowID)
				{
					ostartp = pdp.second.RowIDType.Allocate();
					pdp.second.RowIDType.Decode(range.rowID, ostartp);
					startId = Optional<void*>(ostartp);
				}

				fs::Range frange(query.limit, range.ascending, starto, startId, endo);				
				GetResult result = pdp.first->GetRows(frange);

				if (ostartp != NULL)
					delete ostartp;

				if (oendp != NULL)
					delete oendp;

				fastore::ValueRowsList vrl(result.Data.size());
				fastore::RangeResult rr;

				rr.endOfRange = result.EndOfRange;
				rr.beginOfRange = result.BeginOfRange;
				rr.limited = result.Limited;

				for (int j = 0; j < result.Data.size(); j++ )
				{
					fastore::ValueRows vr;
					fs::ValueKeys vk = result.Data[j];

					pdp.second.ValueType.Encode(vk.first, vr.value);

					vr.rowIDs = std::vector<std::string>(vk.second.size());
					for (int k = 0; k < vk.second.size(); k++)
					{
						std::string id;
						pdp.second.RowIDType.Encode(vk.second[k], vr.rowIDs[k]);
					}

					vrl[j] = vr;
				}

				rr.valueRowsList = vrl;

				res.answer.__isset.rangeValues = true;
				res.answer.rangeValues.push_back(rr);
			}
		}

		if (query.rowIDs.size() > 0)
		{
			fs::KeyVector kv(query.rowIDs.size());
			for (int i = 0; i < query.rowIDs.size(); i++)
			{
				kv[i] = pdp.second.RowIDType.Allocate();
				pdp.second.RowIDType.Decode(query.rowIDs[i], kv[i]);
			}

			auto result = pdp.first->GetValues(kv);

			res.answer.__set_rowIDValues(std::vector<std::string>(result.size()));
			for (int i = 0; i < result.size(); i++)
			{
				pdp.second.ValueType.Encode(result[i], res.answer.rowIDValues[i]);
			}

			//Remove temporarily decoded rowIds.
			for (int i = 0; i < kv.size(); i++)
			{
				delete kv[i];
			}
		}

		_return.insert(std::pair<fastore::ColumnID, fastore::ReadResult>(id, res));
		start++;
	}
}

void WorkerHandler::getStatistics(std::vector<Statistic> & _return, const std::vector<ColumnID> & columnIDs)
{	
	for (int i = 0; i < columnIDs.size(); i++)
	{
		Statistic stat;
		
		auto result = _host.GetColumn(columnIDs[i]).first->GetStatistics();
		stat.total = result.Total;
		stat.unique = result.Unique;

		_return.push_back(stat);
	}
}
