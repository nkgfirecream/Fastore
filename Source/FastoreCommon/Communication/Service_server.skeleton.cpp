// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "Service.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::fastore::communication;

class ServiceHandler : virtual public ServiceIf {
 public:
  ServiceHandler() {
    // Your initialization goes here
  }

  void shutdown() {
    // Your implementation goes here
    printf("shutdown\n");
  }

  void ping() {
    // Your implementation goes here
    printf("ping\n");
  }

  void init(ServiceState& _return, const Topology& topology, const HostAddresses& addresses, const HostID hostID) {
    // Your implementation goes here
    printf("init\n");
  }

  void join(ServiceState& _return, const HiveState& hiveState, const NetworkAddress& address, const HostID hostID) {
    // Your implementation goes here
    printf("join\n");
  }

  void leave() {
    // Your implementation goes here
    printf("leave\n");
  }

  void getHiveState(OptionalHiveState& _return, const bool forceUpdate) {
    // Your implementation goes here
    printf("getHiveState\n");
  }

  void getState(OptionalServiceState& _return) {
    // Your implementation goes here
    printf("getState\n");
  }

  LockID acquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout) {
    // Your implementation goes here
    printf("acquireLock\n");
  }

  void keepLock(const LockID lockID) {
    // Your implementation goes here
    printf("keepLock\n");
  }

  void escalateLock(const LockID lockID, const LockTimeout timeout) {
    // Your implementation goes here
    printf("escalateLock\n");
  }

  void releaseLock(const LockID lockID) {
    // Your implementation goes here
    printf("releaseLock\n");
  }

  void checkpoint(const ColumnIDs& columnIDs) {
    // Your implementation goes here
    printf("checkpoint\n");
  }

  void heartbeat() {
    // Your implementation goes here
    printf("heartbeat\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<ServiceHandler> handler(new ServiceHandler());
  shared_ptr<TProcessor> processor(new ServiceProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

