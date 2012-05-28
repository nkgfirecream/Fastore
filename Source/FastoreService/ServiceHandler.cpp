#include "ServiceHandler.h"

using namespace std;

ServiceHandler::ServiceHandler(const ServiceConfig& config) : _host(config.coreConfig) {
}

void ServiceHandler::getTopology(TopologyResult& _return) {
// Your implementation goes here
printf("GetTopology\n");
}

Revision ServiceHandler::prepareTopology(const TransactionID& transactionID, const Topology& topology) {
// Your implementation goes here
printf("PrepareTopology\n");

return 0;
}

void ServiceHandler::commitTopology(const TransactionID& transactionID) {
// Your implementation goes here
printf("CommitTopology\n");
}

void ServiceHandler::rollbackTopology(const TransactionID& transactionID) {
// Your implementation goes here
printf("RollbackTopology\n");
}

void ServiceHandler::getTopologyReport(TopologyReport& _return) {
// Your implementation goes here
printf("GetTopologyReport\n");
}

void ServiceHandler::getReport(HostReport& _return) {
// Your implementation goes here
printf("GetReport\n");
}

Revision ServiceHandler::prepare(const TransactionID& transactionID, const Writes& writes, const Reads& reads) {
// Your implementation goes here
printf("Prepare\n");

return 0;
}

void ServiceHandler::apply(TransactionID& _return, const TransactionID& transactionID, const Writes& writes) 
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

void ServiceHandler::commit(const TransactionID& transactionID) {
// Your implementation goes here
printf("Commit\n");
}

void ServiceHandler::rollback(const TransactionID& transactionID) {
// Your implementation goes here
printf("Rollback\n");
}

void ServiceHandler::flush(const TransactionID& transactionID) {
// Your implementation goes here
printf("Flush\n");
}

bool ServiceHandler::doesConflict(const Reads& reads, const Revision source, const Revision target) {
// Your implementation goes here
printf("DoesConflict\n");

return false;
}

void ServiceHandler::update(TransactionID& _return, const TransactionID& transactionID, const Writes& writes, const Reads& reads) {
// Your implementation goes here
printf("Update\n");
}

void ServiceHandler::transgrade(Reads& _return, const Reads& reads, const Revision source, const Revision target) {
// Your implementation goes here
printf("Transgrade\n");
}

LockID ServiceHandler::acquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout) {
// Your implementation goes here
printf("AcquireLock\n");

return 0;
}

void ServiceHandler::keepLock(const LockID lockID) {
// Your implementation goes here
printf("KeepLock\n");
}

void ServiceHandler::escalateLock(const LockID lockID, const LockTimeout timeout) {
// Your implementation goes here
printf("EscalateLock\n");
}

void ServiceHandler::releaseLock(const LockID lockID) {
// Your implementation goes here
printf("ReleaseLock\n");
}

void ServiceHandler::query(ReadResults& _return, const Queries& queries)
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

					if (range.first.__isset.rowID)
					{
						ostartp = pdp.second.RowIDType.Allocate();
						pdp.second.RowIDType.Decode(range.first.rowID, ostartp);
						rb.RowId = Optional<void*>(ostartp);
					}

					starto = Optional<fs::RangeBound>(rb);
				}

				if (range.__isset.last)
				{
					fs::RangeBound rb;
					rb.Inclusive = range.last.inclusive;
					rb.Value = pdp.second.ValueType.Allocate();
					pdp.second.ValueType.Decode(range.last.value, rb.Value);
					if (range.last.__isset.rowID)
					{
						oendp = pdp.second.RowIDType.Allocate();
						pdp.second.RowIDType.Decode(range.last.rowID, oendp);
						rb.RowId = Optional<void*>(oendp);
					}

					endo = Optional<fs::RangeBound>(rb);
				}		

				fs::Range frange(range.limit, range.ascending, starto, endo);				
				GetResult result = pdp.first->GetRows(frange);

				if (ostartp != NULL)
					delete ostartp;

				if (oendp != NULL)
					delete oendp;

				fastore::ValueRowsList vrl(result.Data.size());
				fastore::RangeResult rr;

				rr.endOfRange = !result.Limited;

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

			res.answer.rowIDValues = std::vector<std::string>(result.size());
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

		_return.insert(pair<fastore::ColumnID, fastore::ReadResult&>(id, res));
		start++;
	}
}

void ServiceHandler::getStatistics(std::vector<Statistic> & _return, const std::vector<ColumnID> & columnIDs)
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