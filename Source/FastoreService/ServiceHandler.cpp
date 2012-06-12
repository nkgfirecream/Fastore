#include "ServiceHandler.h"

using namespace std;

ServiceHandler::ServiceHandler(const CoreConfig& config) : _host(config)
{
}

void ServiceHandler::init(HiveState& _return) {
// Your implementation goes here
printf("init\n");
}

void ServiceHandler::join(ServiceState& _return, const HostID hostID, const HiveState& hiveState) {
// Your implementation goes here
printf("join\n");
}

void ServiceHandler::leave() {
// Your implementation goes here
printf("leave\n");
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
