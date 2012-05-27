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
  void GetTopology(TopologyResult& _return);
  Revision PrepareTopology(const TransactionID& transactionID, const Topology& topology);
  void CommitTopology(const TransactionID& transactionID);
  void RollbackTopology(const TransactionID& transactionID);
  void GetTopologyReport(TopologyReport& _return);
  void GetReport(HostReport& _return);
  Revision Prepare(const TransactionID& transactionID, const Writes& writes, const Reads& reads);
  void Apply(TransactionID& _return, const TransactionID& transactionID, const Writes& writes);
  void Commit(const TransactionID& transactionID);
  void Rollback(const TransactionID& transactionID);
  void Flush(const TransactionID& transactionID);
  bool DoesConflict(const Reads& reads, const Revision source, const Revision target);
  void Update(TransactionID& _return, const TransactionID& transactionID, const Writes& writes, const Reads& reads);
  void Transgrade(Reads& _return, const Reads& reads, const Revision source, const Revision target);
  LockID AcquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout);
  void KeepLock(const LockID lockID);
  void EscalateLock(const LockID lockID, const LockTimeout timeout);
  void ReleaseLock(const LockID lockID);
  void Query(ReadResults& _return, const Queries& queries);
  void GetStatistics(std::vector<Statistic> & _return, const std::vector<ColumnID> & columnIDs);

};