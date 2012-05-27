#include "ServiceHandler.h"

using namespace std;

ServiceHandler::ServiceHandler(const ServiceConfig& config) : _host(config.coreConfig) {
}

void ServiceHandler::GetTopology(TopologyResult& _return) {
// Your implementation goes here
printf("GetTopology\n");
}

Revision ServiceHandler::PrepareTopology(const TransactionID& transactionID, const Topology& topology) {
// Your implementation goes here
printf("PrepareTopology\n");

return 0;
}

void ServiceHandler::CommitTopology(const TransactionID& transactionID) {
// Your implementation goes here
printf("CommitTopology\n");
}

void ServiceHandler::RollbackTopology(const TransactionID& transactionID) {
// Your implementation goes here
printf("RollbackTopology\n");
}

void ServiceHandler::GetTopologyReport(TopologyReport& _return) {
// Your implementation goes here
printf("GetTopologyReport\n");
}

void ServiceHandler::GetReport(HostReport& _return) {
// Your implementation goes here
printf("GetReport\n");
}

Revision ServiceHandler::Prepare(const TransactionID& transactionID, const Writes& writes, const Reads& reads) {
// Your implementation goes here
printf("Prepare\n");

return 0;
}

void ServiceHandler::Apply(TransactionID& _return, const TransactionID& transactionID, const Writes& writes) 
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

		auto exStart = writes.Excludes.begin();
		while (exStart != writes.Excludes.end())
		{
			//TODO: Fix Leaks
			void* rowIdp = pdp.second.RowIDType.Allocate();			
			pdp.second.RowIDType.Decode((*exStart).RowID, rowIdp);
			pdp.first->Exclude(rowIdp);

			delete rowIdp;
			exStart++;
		}

		auto inStart = writes.Includes.begin();
		while (inStart != writes.Includes.end())
		{
			void* rowIdp = pdp.second.RowIDType.Allocate();
			void* valuep = pdp.second.ValueType.Allocate();

			pdp.second.RowIDType.Decode((*inStart).RowID, rowIdp);
			pdp.second.ValueType.Decode((*inStart).Value, valuep);

			pdp.first->Include(valuep, rowIdp);

			delete valuep, rowIdp;

			inStart++;
		}

		start++;
	}

	if (syncSchema)
		_host.SyncToSchema();
}

void ServiceHandler::Commit(const TransactionID& transactionID) {
// Your implementation goes here
printf("Commit\n");
}

void ServiceHandler::Rollback(const TransactionID& transactionID) {
// Your implementation goes here
printf("Rollback\n");
}

void ServiceHandler::Flush(const TransactionID& transactionID) {
// Your implementation goes here
printf("Flush\n");
}

bool ServiceHandler::DoesConflict(const Reads& reads, const Revision source, const Revision target) {
// Your implementation goes here
printf("DoesConflict\n");

return false;
}

void ServiceHandler::Update(TransactionID& _return, const TransactionID& transactionID, const Writes& writes, const Reads& reads) {
// Your implementation goes here
printf("Update\n");
}

void ServiceHandler::Transgrade(Reads& _return, const Reads& reads, const Revision source, const Revision target) {
// Your implementation goes here
printf("Transgrade\n");
}

LockID ServiceHandler::AcquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout) {
// Your implementation goes here
printf("AcquireLock\n");

return 0;
}

void ServiceHandler::KeepLock(const LockID lockID) {
// Your implementation goes here
printf("KeepLock\n");
}

void ServiceHandler::EscalateLock(const LockID lockID, const LockTimeout timeout) {
// Your implementation goes here
printf("EscalateLock\n");
}

void ServiceHandler::ReleaseLock(const LockID lockID) {
// Your implementation goes here
printf("ReleaseLock\n");
}

void ServiceHandler::Query(ReadResults& _return, const Queries& queries)
{
	auto start = queries.begin();

	while(start != queries.end())
	{
		fastore::ColumnID id = (*start).first;
		fastore::Query query = (*start).second;

		PointerDefPair pdp = _host.GetColumn(id);

		fastore::Answer ans;		

		if (query.Ranges.size() > 0)
		{
			for (int i = 0; i < query.Ranges.size(); i++)
			{
				auto range = query.Ranges[i];

				Optional<fs::RangeBound> starto;
				Optional<fs::RangeBound> endo;

				void* ostartp = NULL;
				void* oendp = NULL;

				if (range.__isset.Start)
				{
					fs::RangeBound rb;
					rb.Inclusive = range.Start.Inclusive;
					rb.Value = pdp.second.ValueType.Allocate();
					pdp.second.ValueType.Decode(range.Start.Value, rb.Value);

					if (range.Start.__isset.RowID)
					{
						ostartp = pdp.second.RowIDType.Allocate();
						pdp.second.RowIDType.Decode(range.Start.RowID, ostartp);
						rb.RowId = Optional<void*>(ostartp);
					}

					starto = Optional<fs::RangeBound>(rb);
				}

				if (range.__isset.End)
				{
					fs::RangeBound rb;
					rb.Inclusive = range.End.Inclusive;
					rb.Value = pdp.second.ValueType.Allocate();
					pdp.second.ValueType.Decode(range.End.Value, rb.Value);
					if (range.End.__isset.RowID)
					{
						oendp = pdp.second.RowIDType.Allocate();
						pdp.second.RowIDType.Decode(range.End.RowID, oendp);
						rb.RowId = Optional<void*>(oendp);
					}

					endo = Optional<fs::RangeBound>(rb);
				}		

				fs::Range frange(range.Limit, range.Ascending, starto, endo);				
				GetResult result = pdp.first->GetRows(frange);

				if (ostartp != NULL)
					delete ostartp;

				if (oendp != NULL)
					delete oendp;

				fastore::ValueRowsList vrl(result.Data.size());
				fastore::RangeResult rr;

				rr.EndOfRange = !result.Limited;

				for (int j = 0; j < result.Data.size(); j++ )
				{
					fastore::ValueRows vr;
					fs::ValueKeys vk = result.Data[j];

					pdp.second.ValueType.Encode(vk.first, vr.Value);

					vr.RowIDs = std::vector<std::string>(vk.second.size());
					for (int k = 0; k < vk.second.size(); k++)
					{
						std::string id;
						pdp.second.RowIDType.Encode(vk.second[k], vr.RowIDs[k]);
					}

					vrl[j] = vr;
				}

				rr.valueRowsList = vrl;

				ans.RangeValues.push_back(rr);
			}
		}

		if (query.RowIDs.size() > 0)
		{
			fs::KeyVector kv(query.RowIDs.size());
			for (int i = 0; i < query.RowIDs.size(); i++)
			{
				kv[i] = pdp.second.RowIDType.Allocate();
				pdp.second.RowIDType.Decode(query.RowIDs[i], kv[i]);
			}

			auto result = pdp.first->GetValues(kv);

			ans.RowIDValues = std::vector<std::string>(result.size());
			for (int i = 0; i < result.size(); i++)
			{
				pdp.second.ValueType.Encode(result[i], ans.RowIDValues[i]);
			}

			//Remove temporarily decoded rowIds.
			for (int i = 0; i < kv.size(); i++)
			{
				delete kv[i];
			}
		}

		_return.Answers.insert(pair<fastore::ColumnID, fastore::Answer>(id, ans));
		start++;
	}
}

void ServiceHandler::GetStatistics(std::vector<Statistic> & _return, const std::vector<ColumnID> & columnIDs)
{	
	for (int i = 0; i < columnIDs.size(); i++)
	{
		Statistic stat;
		
		auto result = _host.GetColumn(columnIDs[i]).first->GetStatistics();
		stat.Total = result.Total;
		stat.Unique = result.Unique;

		_return.push_back(stat);
	}
}