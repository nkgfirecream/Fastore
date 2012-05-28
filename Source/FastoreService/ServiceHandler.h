#pragma once
#include "stdafx.h"
#include "..\FastoreCore\FastoreHost.h"
#include "..\FastoreCommunication\Service.h"
#include "ServiceConfig.h"
#include <thrift/server/TServer.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::fastore;

//TODO: Need to cleanly separate different ideas in the server/service.
//We have a Service in c++ which installs Fastore as a service, which is a combination
// of a FastoreHost, a ServiceHandler, a Thrift server, and a c++ ServiceHost.
class ServiceHandler : virtual public ServiceIf {
 private: 
	 FastoreHost _host;

 public:
  ServiceHandler(const ServiceConfig& config);
  void getTopology(TopologyResult& _return);
  Revision prepareTopology(const TransactionID& transactionID, const Topology& topology);
  void commitTopology(const TransactionID& transactionID);
  void rollbackTopology(const TransactionID& transactionID);
  void getTopologyReport(TopologyReport& _return);
  void getReport(HostReport& _return);
  Revision prepare(const TransactionID& transactionID, const Writes& writes, const Reads& reads);
  void apply(TransactionID& _return, const TransactionID& transactionID, const Writes& writes);
  void commit(const TransactionID& transactionID);
  void rollback(const TransactionID& transactionID);
  void flush(const TransactionID& transactionID);
  bool doesConflict(const Reads& reads, const Revision source, const Revision target);
  void update(TransactionID& _return, const TransactionID& transactionID, const Writes& writes, const Reads& reads);
  void transgrade(Reads& _return, const Reads& reads, const Revision source, const Revision target);
  LockID acquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout);
  void keepLock(const LockID lockID);
  void escalateLock(const LockID lockID, const LockTimeout timeout);
  void releaseLock(const LockID lockID);
  void query(ReadResults& _return, const Queries& queries);
  void getStatistics(std::vector<Statistic> & _return, const std::vector<ColumnID> & columnIDs);

};