#include "ServiceHandler.h"
ServiceHandler::ServiceHandler() {
// Your initialization goes here
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

void ServiceHandler::Apply(TransactionID& _return, const TransactionID& transactionID, const Writes& writes) {
// Your implementation goes here
printf("Apply\n");
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

void ServiceHandler::Query(ReadResults& _return, const Queries& queries) {
// Your implementation goes here
printf("Query\n");
}

void ServiceHandler::GetStatistics(std::vector<Statistic> & _return, const std::vector<ColumnID> & columnIDs) {
// Your implementation goes here
printf("GetStatistics\n");
}