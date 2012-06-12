#pragma once
#include "..\FastoreCore\FastoreHost.h"
#include "..\FastoreCommunication\Service.h"
#include <thrift/server/TServer.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::fastore;

class ServiceHandler : virtual public ServiceIf 
{
private: 
	FastoreHost _host;

public:
	ServiceHandler(const CoreConfig& config);

	void init(HiveState& _return);
	void join(ServiceState& _return, const HostID hostID, const HiveState& hiveState);
	void leave();
	void getHiveState(HiveState& _return);
	void getState(ServiceState& _return);
	LockID acquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout);
	void keepLock(const LockID lockID);
	void escalateLock(const LockID lockID, const LockTimeout timeout);
	void releaseLock(const LockID lockID);
};