#include "ServiceHandler.h"

using namespace std;

ServiceHandler::ServiceHandler(const ServiceConfig& config) : _host(config.coreConfig)
{
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

void ServiceHandler::getHiveState(HiveState& _return) {
// Your implementation goes here
printf("GetHiveState\n");
}

void ServiceHandler::getState(ServiceState& _return) {
// Your implementation goes here
printf("GetState\n");
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
