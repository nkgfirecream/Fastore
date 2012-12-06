/**
 * Autogenerated by Thrift Compiler (0.9.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef Store_H
#define Store_H

#include <thrift/TDispatchProcessor.h>
#include "Comm_types.h"

namespace fastore { namespace communication {

class StoreIf {
 public:
  virtual ~StoreIf() {}
  virtual void checkpointBegin(const ColumnID columnID) = 0;
  virtual void checkpointWrite(const ColumnID columnID, const ValueRowsList& values) = 0;
  virtual void checkpointEnd(const ColumnID columnID) = 0;
  virtual void getStatus(StoreStatus& _return) = 0;
  virtual void getWrites(GetWritesResults& _return, const Ranges& ranges) = 0;
  virtual void commit(const TransactionID transactionID, const Writes& writes) = 0;
  virtual void flush(const TransactionID transactionID) = 0;
};

class StoreIfFactory {
 public:
  typedef StoreIf Handler;

  virtual ~StoreIfFactory() {}

  virtual StoreIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(StoreIf* /* handler */) = 0;
};

class StoreIfSingletonFactory : virtual public StoreIfFactory {
 public:
  StoreIfSingletonFactory(const boost::shared_ptr<StoreIf>& iface) : iface_(iface) {}
  virtual ~StoreIfSingletonFactory() {}

  virtual StoreIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(StoreIf* /* handler */) {}

 protected:
  boost::shared_ptr<StoreIf> iface_;
};

class StoreNull : virtual public StoreIf {
 public:
  virtual ~StoreNull() {}
  void checkpointBegin(const ColumnID /* columnID */) {
    return;
  }
  void checkpointWrite(const ColumnID /* columnID */, const ValueRowsList& /* values */) {
    return;
  }
  void checkpointEnd(const ColumnID /* columnID */) {
    return;
  }
  void getStatus(StoreStatus& /* _return */) {
    return;
  }
  void getWrites(GetWritesResults& /* _return */, const Ranges& /* ranges */) {
    return;
  }
  void commit(const TransactionID /* transactionID */, const Writes& /* writes */) {
    return;
  }
  void flush(const TransactionID /* transactionID */) {
    return;
  }
};

typedef struct _Store_checkpointBegin_args__isset {
  _Store_checkpointBegin_args__isset() : columnID(false) {}
  bool columnID;
} _Store_checkpointBegin_args__isset;

class Store_checkpointBegin_args {
 public:

  Store_checkpointBegin_args() : columnID(0) {
  }

  virtual ~Store_checkpointBegin_args() throw() {}

  ColumnID columnID;

  _Store_checkpointBegin_args__isset __isset;

  void __set_columnID(const ColumnID val) {
    columnID = val;
  }

  bool operator == (const Store_checkpointBegin_args & rhs) const
  {
    if (!(columnID == rhs.columnID))
      return false;
    return true;
  }
  bool operator != (const Store_checkpointBegin_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_checkpointBegin_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_checkpointBegin_pargs {
 public:


  virtual ~Store_checkpointBegin_pargs() throw() {}

  const ColumnID* columnID;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_checkpointBegin_result {
 public:

  Store_checkpointBegin_result() {
  }

  virtual ~Store_checkpointBegin_result() throw() {}


  bool operator == (const Store_checkpointBegin_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Store_checkpointBegin_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_checkpointBegin_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_checkpointBegin_presult {
 public:


  virtual ~Store_checkpointBegin_presult() throw() {}


  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _Store_checkpointWrite_args__isset {
  _Store_checkpointWrite_args__isset() : columnID(false), values(false) {}
  bool columnID;
  bool values;
} _Store_checkpointWrite_args__isset;

class Store_checkpointWrite_args {
 public:

  Store_checkpointWrite_args() : columnID(0) {
  }

  virtual ~Store_checkpointWrite_args() throw() {}

  ColumnID columnID;
  ValueRowsList values;

  _Store_checkpointWrite_args__isset __isset;

  void __set_columnID(const ColumnID val) {
    columnID = val;
  }

  void __set_values(const ValueRowsList& val) {
    values = val;
  }

  bool operator == (const Store_checkpointWrite_args & rhs) const
  {
    if (!(columnID == rhs.columnID))
      return false;
    if (!(values == rhs.values))
      return false;
    return true;
  }
  bool operator != (const Store_checkpointWrite_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_checkpointWrite_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_checkpointWrite_pargs {
 public:


  virtual ~Store_checkpointWrite_pargs() throw() {}

  const ColumnID* columnID;
  const ValueRowsList* values;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_checkpointWrite_result {
 public:

  Store_checkpointWrite_result() {
  }

  virtual ~Store_checkpointWrite_result() throw() {}


  bool operator == (const Store_checkpointWrite_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Store_checkpointWrite_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_checkpointWrite_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_checkpointWrite_presult {
 public:


  virtual ~Store_checkpointWrite_presult() throw() {}


  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _Store_checkpointEnd_args__isset {
  _Store_checkpointEnd_args__isset() : columnID(false) {}
  bool columnID;
} _Store_checkpointEnd_args__isset;

class Store_checkpointEnd_args {
 public:

  Store_checkpointEnd_args() : columnID(0) {
  }

  virtual ~Store_checkpointEnd_args() throw() {}

  ColumnID columnID;

  _Store_checkpointEnd_args__isset __isset;

  void __set_columnID(const ColumnID val) {
    columnID = val;
  }

  bool operator == (const Store_checkpointEnd_args & rhs) const
  {
    if (!(columnID == rhs.columnID))
      return false;
    return true;
  }
  bool operator != (const Store_checkpointEnd_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_checkpointEnd_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_checkpointEnd_pargs {
 public:


  virtual ~Store_checkpointEnd_pargs() throw() {}

  const ColumnID* columnID;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_checkpointEnd_result {
 public:

  Store_checkpointEnd_result() {
  }

  virtual ~Store_checkpointEnd_result() throw() {}


  bool operator == (const Store_checkpointEnd_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Store_checkpointEnd_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_checkpointEnd_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_checkpointEnd_presult {
 public:


  virtual ~Store_checkpointEnd_presult() throw() {}


  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};


class Store_getStatus_args {
 public:

  Store_getStatus_args() {
  }

  virtual ~Store_getStatus_args() throw() {}


  bool operator == (const Store_getStatus_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Store_getStatus_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_getStatus_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_getStatus_pargs {
 public:


  virtual ~Store_getStatus_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Store_getStatus_result__isset {
  _Store_getStatus_result__isset() : success(false) {}
  bool success;
} _Store_getStatus_result__isset;

class Store_getStatus_result {
 public:

  Store_getStatus_result() {
  }

  virtual ~Store_getStatus_result() throw() {}

  StoreStatus success;

  _Store_getStatus_result__isset __isset;

  void __set_success(const StoreStatus& val) {
    success = val;
  }

  bool operator == (const Store_getStatus_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const Store_getStatus_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_getStatus_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Store_getStatus_presult__isset {
  _Store_getStatus_presult__isset() : success(false) {}
  bool success;
} _Store_getStatus_presult__isset;

class Store_getStatus_presult {
 public:


  virtual ~Store_getStatus_presult() throw() {}

  StoreStatus* success;

  _Store_getStatus_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _Store_getWrites_args__isset {
  _Store_getWrites_args__isset() : ranges(false) {}
  bool ranges;
} _Store_getWrites_args__isset;

class Store_getWrites_args {
 public:

  Store_getWrites_args() {
  }

  virtual ~Store_getWrites_args() throw() {}

  Ranges ranges;

  _Store_getWrites_args__isset __isset;

  void __set_ranges(const Ranges& val) {
    ranges = val;
  }

  bool operator == (const Store_getWrites_args & rhs) const
  {
    if (!(ranges == rhs.ranges))
      return false;
    return true;
  }
  bool operator != (const Store_getWrites_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_getWrites_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_getWrites_pargs {
 public:


  virtual ~Store_getWrites_pargs() throw() {}

  const Ranges* ranges;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Store_getWrites_result__isset {
  _Store_getWrites_result__isset() : success(false) {}
  bool success;
} _Store_getWrites_result__isset;

class Store_getWrites_result {
 public:

  Store_getWrites_result() {
  }

  virtual ~Store_getWrites_result() throw() {}

  GetWritesResults success;

  _Store_getWrites_result__isset __isset;

  void __set_success(const GetWritesResults& val) {
    success = val;
  }

  bool operator == (const Store_getWrites_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const Store_getWrites_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_getWrites_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Store_getWrites_presult__isset {
  _Store_getWrites_presult__isset() : success(false) {}
  bool success;
} _Store_getWrites_presult__isset;

class Store_getWrites_presult {
 public:


  virtual ~Store_getWrites_presult() throw() {}

  GetWritesResults* success;

  _Store_getWrites_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _Store_commit_args__isset {
  _Store_commit_args__isset() : transactionID(false), writes(false) {}
  bool transactionID;
  bool writes;
} _Store_commit_args__isset;

class Store_commit_args {
 public:

  Store_commit_args() : transactionID(0) {
  }

  virtual ~Store_commit_args() throw() {}

  TransactionID transactionID;
  Writes writes;

  _Store_commit_args__isset __isset;

  void __set_transactionID(const TransactionID val) {
    transactionID = val;
  }

  void __set_writes(const Writes& val) {
    writes = val;
  }

  bool operator == (const Store_commit_args & rhs) const
  {
    if (!(transactionID == rhs.transactionID))
      return false;
    if (!(writes == rhs.writes))
      return false;
    return true;
  }
  bool operator != (const Store_commit_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_commit_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_commit_pargs {
 public:


  virtual ~Store_commit_pargs() throw() {}

  const TransactionID* transactionID;
  const Writes* writes;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Store_flush_args__isset {
  _Store_flush_args__isset() : transactionID(false) {}
  bool transactionID;
} _Store_flush_args__isset;

class Store_flush_args {
 public:

  Store_flush_args() : transactionID(0) {
  }

  virtual ~Store_flush_args() throw() {}

  TransactionID transactionID;

  _Store_flush_args__isset __isset;

  void __set_transactionID(const TransactionID val) {
    transactionID = val;
  }

  bool operator == (const Store_flush_args & rhs) const
  {
    if (!(transactionID == rhs.transactionID))
      return false;
    return true;
  }
  bool operator != (const Store_flush_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_flush_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_flush_pargs {
 public:


  virtual ~Store_flush_pargs() throw() {}

  const TransactionID* transactionID;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_flush_result {
 public:

  Store_flush_result() {
  }

  virtual ~Store_flush_result() throw() {}


  bool operator == (const Store_flush_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Store_flush_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Store_flush_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Store_flush_presult {
 public:


  virtual ~Store_flush_presult() throw() {}


  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class StoreClient : virtual public StoreIf {
 public:
  StoreClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
    piprot_(prot),
    poprot_(prot) {
    iprot_ = prot.get();
    oprot_ = prot.get();
  }
  StoreClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) :
    piprot_(iprot),
    poprot_(oprot) {
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void checkpointBegin(const ColumnID columnID);
  void send_checkpointBegin(const ColumnID columnID);
  void recv_checkpointBegin();
  void checkpointWrite(const ColumnID columnID, const ValueRowsList& values);
  void send_checkpointWrite(const ColumnID columnID, const ValueRowsList& values);
  void recv_checkpointWrite();
  void checkpointEnd(const ColumnID columnID);
  void send_checkpointEnd(const ColumnID columnID);
  void recv_checkpointEnd();
  void getStatus(StoreStatus& _return);
  void send_getStatus();
  void recv_getStatus(StoreStatus& _return);
  void getWrites(GetWritesResults& _return, const Ranges& ranges);
  void send_getWrites(const Ranges& ranges);
  void recv_getWrites(GetWritesResults& _return);
  void commit(const TransactionID transactionID, const Writes& writes);
  void send_commit(const TransactionID transactionID, const Writes& writes);
  void flush(const TransactionID transactionID);
  void send_flush(const TransactionID transactionID);
  void recv_flush();
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class StoreProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<StoreIf> iface_;
  virtual bool dispatchCall(apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (StoreProcessor::*ProcessFunction)(int32_t, apache::thrift::protocol::TProtocol*, apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_checkpointBegin(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_checkpointWrite(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_checkpointEnd(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_getStatus(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_getWrites(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_commit(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_flush(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  StoreProcessor(boost::shared_ptr<StoreIf> iface) :
    iface_(iface) {
    processMap_["checkpointBegin"] = &StoreProcessor::process_checkpointBegin;
    processMap_["checkpointWrite"] = &StoreProcessor::process_checkpointWrite;
    processMap_["checkpointEnd"] = &StoreProcessor::process_checkpointEnd;
    processMap_["getStatus"] = &StoreProcessor::process_getStatus;
    processMap_["getWrites"] = &StoreProcessor::process_getWrites;
    processMap_["commit"] = &StoreProcessor::process_commit;
    processMap_["flush"] = &StoreProcessor::process_flush;
  }

  virtual ~StoreProcessor() {}
};

class StoreProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  StoreProcessorFactory(const ::boost::shared_ptr< StoreIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< StoreIfFactory > handlerFactory_;
};

class StoreMultiface : virtual public StoreIf {
 public:
  StoreMultiface(std::vector<boost::shared_ptr<StoreIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~StoreMultiface() {}
 protected:
  std::vector<boost::shared_ptr<StoreIf> > ifaces_;
  StoreMultiface() {}
  void add(boost::shared_ptr<StoreIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void checkpointBegin(const ColumnID columnID) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->checkpointBegin(columnID);
    }
    ifaces_[i]->checkpointBegin(columnID);
  }

  void checkpointWrite(const ColumnID columnID, const ValueRowsList& values) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->checkpointWrite(columnID, values);
    }
    ifaces_[i]->checkpointWrite(columnID, values);
  }

  void checkpointEnd(const ColumnID columnID) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->checkpointEnd(columnID);
    }
    ifaces_[i]->checkpointEnd(columnID);
  }

  void getStatus(StoreStatus& _return) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->getStatus(_return);
    }
    ifaces_[i]->getStatus(_return);
    return;
  }

  void getWrites(GetWritesResults& _return, const Ranges& ranges) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->getWrites(_return, ranges);
    }
    ifaces_[i]->getWrites(_return, ranges);
    return;
  }

  void commit(const TransactionID transactionID, const Writes& writes) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->commit(transactionID, writes);
    }
    ifaces_[i]->commit(transactionID, writes);
  }

  void flush(const TransactionID transactionID) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->flush(transactionID);
    }
    ifaces_[i]->flush(transactionID);
  }

};

}} // namespace

#endif
