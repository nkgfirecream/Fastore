/**
 * Autogenerated by Thrift Compiler (0.9.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef Service_H
#define Service_H

#include <thrift/TDispatchProcessor.h>
#include "Fastore_types.h"

namespace fastore {

class ServiceIf {
 public:
  virtual ~ServiceIf() {}
  virtual void init(HiveState& _return) = 0;
  virtual void join(ServiceState& _return, const HostID hostID, const HiveState& hiveState) = 0;
  virtual void leave() = 0;
  virtual void getHiveState(HiveState& _return) = 0;
  virtual void getState(ServiceState& _return) = 0;
  virtual LockID acquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout) = 0;
  virtual void keepLock(const LockID lockID) = 0;
  virtual void escalateLock(const LockID lockID, const LockTimeout timeout) = 0;
  virtual void releaseLock(const LockID lockID) = 0;
};

class ServiceIfFactory {
 public:
  typedef ServiceIf Handler;

  virtual ~ServiceIfFactory() {}

  virtual ServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(ServiceIf* /* handler */) = 0;
};

class ServiceIfSingletonFactory : virtual public ServiceIfFactory {
 public:
  ServiceIfSingletonFactory(const boost::shared_ptr<ServiceIf>& iface) : iface_(iface) {}
  virtual ~ServiceIfSingletonFactory() {}

  virtual ServiceIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(ServiceIf* /* handler */) {}

 protected:
  boost::shared_ptr<ServiceIf> iface_;
};

class ServiceNull : virtual public ServiceIf {
 public:
  virtual ~ServiceNull() {}
  void init(HiveState& /* _return */) {
    return;
  }
  void join(ServiceState& /* _return */, const HostID /* hostID */, const HiveState& /* hiveState */) {
    return;
  }
  void leave() {
    return;
  }
  void getHiveState(HiveState& /* _return */) {
    return;
  }
  void getState(ServiceState& /* _return */) {
    return;
  }
  LockID acquireLock(const LockName& /* name */, const LockMode::type /* mode */, const LockTimeout /* timeout */) {
    LockID _return = 0;
    return _return;
  }
  void keepLock(const LockID /* lockID */) {
    return;
  }
  void escalateLock(const LockID /* lockID */, const LockTimeout /* timeout */) {
    return;
  }
  void releaseLock(const LockID /* lockID */) {
    return;
  }
};


class Service_init_args {
 public:

  Service_init_args() {
  }

  virtual ~Service_init_args() throw() {}


  bool operator == (const Service_init_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Service_init_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_init_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Service_init_pargs {
 public:


  virtual ~Service_init_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_init_result__isset {
  _Service_init_result__isset() : success(false), alreadyJoined(false) {}
  bool success;
  bool alreadyJoined;
} _Service_init_result__isset;

class Service_init_result {
 public:

  Service_init_result() {
  }

  virtual ~Service_init_result() throw() {}

  HiveState success;
  AlreadyJoined alreadyJoined;

  _Service_init_result__isset __isset;

  void __set_success(const HiveState& val) {
    success = val;
  }

  void __set_alreadyJoined(const AlreadyJoined& val) {
    alreadyJoined = val;
  }

  bool operator == (const Service_init_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(alreadyJoined == rhs.alreadyJoined))
      return false;
    return true;
  }
  bool operator != (const Service_init_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_init_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_init_presult__isset {
  _Service_init_presult__isset() : success(false), alreadyJoined(false) {}
  bool success;
  bool alreadyJoined;
} _Service_init_presult__isset;

class Service_init_presult {
 public:


  virtual ~Service_init_presult() throw() {}

  HiveState* success;
  AlreadyJoined alreadyJoined;

  _Service_init_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _Service_join_args__isset {
  _Service_join_args__isset() : hostID(false), hiveState(false) {}
  bool hostID;
  bool hiveState;
} _Service_join_args__isset;

class Service_join_args {
 public:

  Service_join_args() : hostID(0) {
  }

  virtual ~Service_join_args() throw() {}

  HostID hostID;
  HiveState hiveState;

  _Service_join_args__isset __isset;

  void __set_hostID(const HostID val) {
    hostID = val;
  }

  void __set_hiveState(const HiveState& val) {
    hiveState = val;
  }

  bool operator == (const Service_join_args & rhs) const
  {
    if (!(hostID == rhs.hostID))
      return false;
    if (!(hiveState == rhs.hiveState))
      return false;
    return true;
  }
  bool operator != (const Service_join_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_join_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Service_join_pargs {
 public:


  virtual ~Service_join_pargs() throw() {}

  const HostID* hostID;
  const HiveState* hiveState;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_join_result__isset {
  _Service_join_result__isset() : success(false), alreadyJoined(false) {}
  bool success;
  bool alreadyJoined;
} _Service_join_result__isset;

class Service_join_result {
 public:

  Service_join_result() {
  }

  virtual ~Service_join_result() throw() {}

  ServiceState success;
  AlreadyJoined alreadyJoined;

  _Service_join_result__isset __isset;

  void __set_success(const ServiceState& val) {
    success = val;
  }

  void __set_alreadyJoined(const AlreadyJoined& val) {
    alreadyJoined = val;
  }

  bool operator == (const Service_join_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(alreadyJoined == rhs.alreadyJoined))
      return false;
    return true;
  }
  bool operator != (const Service_join_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_join_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_join_presult__isset {
  _Service_join_presult__isset() : success(false), alreadyJoined(false) {}
  bool success;
  bool alreadyJoined;
} _Service_join_presult__isset;

class Service_join_presult {
 public:


  virtual ~Service_join_presult() throw() {}

  ServiceState* success;
  AlreadyJoined alreadyJoined;

  _Service_join_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};


class Service_leave_args {
 public:

  Service_leave_args() {
  }

  virtual ~Service_leave_args() throw() {}


  bool operator == (const Service_leave_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Service_leave_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_leave_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Service_leave_pargs {
 public:


  virtual ~Service_leave_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_leave_result__isset {
  _Service_leave_result__isset() : notJoined(false) {}
  bool notJoined;
} _Service_leave_result__isset;

class Service_leave_result {
 public:

  Service_leave_result() {
  }

  virtual ~Service_leave_result() throw() {}

  NotJoined notJoined;

  _Service_leave_result__isset __isset;

  void __set_notJoined(const NotJoined& val) {
    notJoined = val;
  }

  bool operator == (const Service_leave_result & rhs) const
  {
    if (!(notJoined == rhs.notJoined))
      return false;
    return true;
  }
  bool operator != (const Service_leave_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_leave_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_leave_presult__isset {
  _Service_leave_presult__isset() : notJoined(false) {}
  bool notJoined;
} _Service_leave_presult__isset;

class Service_leave_presult {
 public:


  virtual ~Service_leave_presult() throw() {}

  NotJoined notJoined;

  _Service_leave_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};


class Service_getHiveState_args {
 public:

  Service_getHiveState_args() {
  }

  virtual ~Service_getHiveState_args() throw() {}


  bool operator == (const Service_getHiveState_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Service_getHiveState_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_getHiveState_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Service_getHiveState_pargs {
 public:


  virtual ~Service_getHiveState_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_getHiveState_result__isset {
  _Service_getHiveState_result__isset() : success(false), notJoined(false) {}
  bool success;
  bool notJoined;
} _Service_getHiveState_result__isset;

class Service_getHiveState_result {
 public:

  Service_getHiveState_result() {
  }

  virtual ~Service_getHiveState_result() throw() {}

  HiveState success;
  NotJoined notJoined;

  _Service_getHiveState_result__isset __isset;

  void __set_success(const HiveState& val) {
    success = val;
  }

  void __set_notJoined(const NotJoined& val) {
    notJoined = val;
  }

  bool operator == (const Service_getHiveState_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(notJoined == rhs.notJoined))
      return false;
    return true;
  }
  bool operator != (const Service_getHiveState_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_getHiveState_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_getHiveState_presult__isset {
  _Service_getHiveState_presult__isset() : success(false), notJoined(false) {}
  bool success;
  bool notJoined;
} _Service_getHiveState_presult__isset;

class Service_getHiveState_presult {
 public:


  virtual ~Service_getHiveState_presult() throw() {}

  HiveState* success;
  NotJoined notJoined;

  _Service_getHiveState_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};


class Service_getState_args {
 public:

  Service_getState_args() {
  }

  virtual ~Service_getState_args() throw() {}


  bool operator == (const Service_getState_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Service_getState_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_getState_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Service_getState_pargs {
 public:


  virtual ~Service_getState_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_getState_result__isset {
  _Service_getState_result__isset() : success(false), notJoined(false) {}
  bool success;
  bool notJoined;
} _Service_getState_result__isset;

class Service_getState_result {
 public:

  Service_getState_result() {
  }

  virtual ~Service_getState_result() throw() {}

  ServiceState success;
  NotJoined notJoined;

  _Service_getState_result__isset __isset;

  void __set_success(const ServiceState& val) {
    success = val;
  }

  void __set_notJoined(const NotJoined& val) {
    notJoined = val;
  }

  bool operator == (const Service_getState_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(notJoined == rhs.notJoined))
      return false;
    return true;
  }
  bool operator != (const Service_getState_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_getState_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_getState_presult__isset {
  _Service_getState_presult__isset() : success(false), notJoined(false) {}
  bool success;
  bool notJoined;
} _Service_getState_presult__isset;

class Service_getState_presult {
 public:


  virtual ~Service_getState_presult() throw() {}

  ServiceState* success;
  NotJoined notJoined;

  _Service_getState_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _Service_acquireLock_args__isset {
  _Service_acquireLock_args__isset() : name(false), mode(false), timeout(true) {}
  bool name;
  bool mode;
  bool timeout;
} _Service_acquireLock_args__isset;

class Service_acquireLock_args {
 public:

  Service_acquireLock_args() : name(), mode((LockMode::type)0), timeout(1000) {
  }

  virtual ~Service_acquireLock_args() throw() {}

  LockName name;
  LockMode::type mode;
  LockTimeout timeout;

  _Service_acquireLock_args__isset __isset;

  void __set_name(const LockName& val) {
    name = val;
  }

  void __set_mode(const LockMode::type val) {
    mode = val;
  }

  void __set_timeout(const LockTimeout val) {
    timeout = val;
  }

  bool operator == (const Service_acquireLock_args & rhs) const
  {
    if (!(name == rhs.name))
      return false;
    if (!(mode == rhs.mode))
      return false;
    if (!(timeout == rhs.timeout))
      return false;
    return true;
  }
  bool operator != (const Service_acquireLock_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_acquireLock_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Service_acquireLock_pargs {
 public:


  virtual ~Service_acquireLock_pargs() throw() {}

  const LockName* name;
  const LockMode::type* mode;
  const LockTimeout* timeout;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_acquireLock_result__isset {
  _Service_acquireLock_result__isset() : success(false), timeout(false), notJoined(false) {}
  bool success;
  bool timeout;
  bool notJoined;
} _Service_acquireLock_result__isset;

class Service_acquireLock_result {
 public:

  Service_acquireLock_result() : success(0) {
  }

  virtual ~Service_acquireLock_result() throw() {}

  LockID success;
  LockTimedOut timeout;
  NotJoined notJoined;

  _Service_acquireLock_result__isset __isset;

  void __set_success(const LockID val) {
    success = val;
  }

  void __set_timeout(const LockTimedOut& val) {
    timeout = val;
  }

  void __set_notJoined(const NotJoined& val) {
    notJoined = val;
  }

  bool operator == (const Service_acquireLock_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(timeout == rhs.timeout))
      return false;
    if (!(notJoined == rhs.notJoined))
      return false;
    return true;
  }
  bool operator != (const Service_acquireLock_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_acquireLock_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_acquireLock_presult__isset {
  _Service_acquireLock_presult__isset() : success(false), timeout(false), notJoined(false) {}
  bool success;
  bool timeout;
  bool notJoined;
} _Service_acquireLock_presult__isset;

class Service_acquireLock_presult {
 public:


  virtual ~Service_acquireLock_presult() throw() {}

  LockID* success;
  LockTimedOut timeout;
  NotJoined notJoined;

  _Service_acquireLock_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _Service_keepLock_args__isset {
  _Service_keepLock_args__isset() : lockID(false) {}
  bool lockID;
} _Service_keepLock_args__isset;

class Service_keepLock_args {
 public:

  Service_keepLock_args() : lockID(0) {
  }

  virtual ~Service_keepLock_args() throw() {}

  LockID lockID;

  _Service_keepLock_args__isset __isset;

  void __set_lockID(const LockID val) {
    lockID = val;
  }

  bool operator == (const Service_keepLock_args & rhs) const
  {
    if (!(lockID == rhs.lockID))
      return false;
    return true;
  }
  bool operator != (const Service_keepLock_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_keepLock_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Service_keepLock_pargs {
 public:


  virtual ~Service_keepLock_pargs() throw() {}

  const LockID* lockID;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_keepLock_result__isset {
  _Service_keepLock_result__isset() : expired(false), notJoined(false) {}
  bool expired;
  bool notJoined;
} _Service_keepLock_result__isset;

class Service_keepLock_result {
 public:

  Service_keepLock_result() {
  }

  virtual ~Service_keepLock_result() throw() {}

  LockExpired expired;
  NotJoined notJoined;

  _Service_keepLock_result__isset __isset;

  void __set_expired(const LockExpired& val) {
    expired = val;
  }

  void __set_notJoined(const NotJoined& val) {
    notJoined = val;
  }

  bool operator == (const Service_keepLock_result & rhs) const
  {
    if (!(expired == rhs.expired))
      return false;
    if (!(notJoined == rhs.notJoined))
      return false;
    return true;
  }
  bool operator != (const Service_keepLock_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_keepLock_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_keepLock_presult__isset {
  _Service_keepLock_presult__isset() : expired(false), notJoined(false) {}
  bool expired;
  bool notJoined;
} _Service_keepLock_presult__isset;

class Service_keepLock_presult {
 public:


  virtual ~Service_keepLock_presult() throw() {}

  LockExpired expired;
  NotJoined notJoined;

  _Service_keepLock_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _Service_escalateLock_args__isset {
  _Service_escalateLock_args__isset() : lockID(false), timeout(true) {}
  bool lockID;
  bool timeout;
} _Service_escalateLock_args__isset;

class Service_escalateLock_args {
 public:

  Service_escalateLock_args() : lockID(0), timeout(-1) {
  }

  virtual ~Service_escalateLock_args() throw() {}

  LockID lockID;
  LockTimeout timeout;

  _Service_escalateLock_args__isset __isset;

  void __set_lockID(const LockID val) {
    lockID = val;
  }

  void __set_timeout(const LockTimeout val) {
    timeout = val;
  }

  bool operator == (const Service_escalateLock_args & rhs) const
  {
    if (!(lockID == rhs.lockID))
      return false;
    if (!(timeout == rhs.timeout))
      return false;
    return true;
  }
  bool operator != (const Service_escalateLock_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_escalateLock_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Service_escalateLock_pargs {
 public:


  virtual ~Service_escalateLock_pargs() throw() {}

  const LockID* lockID;
  const LockTimeout* timeout;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_escalateLock_result__isset {
  _Service_escalateLock_result__isset() : timeout(false), expired(false), notJoined(false) {}
  bool timeout;
  bool expired;
  bool notJoined;
} _Service_escalateLock_result__isset;

class Service_escalateLock_result {
 public:

  Service_escalateLock_result() {
  }

  virtual ~Service_escalateLock_result() throw() {}

  LockTimedOut timeout;
  LockExpired expired;
  NotJoined notJoined;

  _Service_escalateLock_result__isset __isset;

  void __set_timeout(const LockTimedOut& val) {
    timeout = val;
  }

  void __set_expired(const LockExpired& val) {
    expired = val;
  }

  void __set_notJoined(const NotJoined& val) {
    notJoined = val;
  }

  bool operator == (const Service_escalateLock_result & rhs) const
  {
    if (!(timeout == rhs.timeout))
      return false;
    if (!(expired == rhs.expired))
      return false;
    if (!(notJoined == rhs.notJoined))
      return false;
    return true;
  }
  bool operator != (const Service_escalateLock_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_escalateLock_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_escalateLock_presult__isset {
  _Service_escalateLock_presult__isset() : timeout(false), expired(false), notJoined(false) {}
  bool timeout;
  bool expired;
  bool notJoined;
} _Service_escalateLock_presult__isset;

class Service_escalateLock_presult {
 public:


  virtual ~Service_escalateLock_presult() throw() {}

  LockTimedOut timeout;
  LockExpired expired;
  NotJoined notJoined;

  _Service_escalateLock_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _Service_releaseLock_args__isset {
  _Service_releaseLock_args__isset() : lockID(false) {}
  bool lockID;
} _Service_releaseLock_args__isset;

class Service_releaseLock_args {
 public:

  Service_releaseLock_args() : lockID(0) {
  }

  virtual ~Service_releaseLock_args() throw() {}

  LockID lockID;

  _Service_releaseLock_args__isset __isset;

  void __set_lockID(const LockID val) {
    lockID = val;
  }

  bool operator == (const Service_releaseLock_args & rhs) const
  {
    if (!(lockID == rhs.lockID))
      return false;
    return true;
  }
  bool operator != (const Service_releaseLock_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_releaseLock_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Service_releaseLock_pargs {
 public:


  virtual ~Service_releaseLock_pargs() throw() {}

  const LockID* lockID;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_releaseLock_result__isset {
  _Service_releaseLock_result__isset() : expired(false), notJoined(false) {}
  bool expired;
  bool notJoined;
} _Service_releaseLock_result__isset;

class Service_releaseLock_result {
 public:

  Service_releaseLock_result() {
  }

  virtual ~Service_releaseLock_result() throw() {}

  LockExpired expired;
  NotJoined notJoined;

  _Service_releaseLock_result__isset __isset;

  void __set_expired(const LockExpired& val) {
    expired = val;
  }

  void __set_notJoined(const NotJoined& val) {
    notJoined = val;
  }

  bool operator == (const Service_releaseLock_result & rhs) const
  {
    if (!(expired == rhs.expired))
      return false;
    if (!(notJoined == rhs.notJoined))
      return false;
    return true;
  }
  bool operator != (const Service_releaseLock_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Service_releaseLock_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Service_releaseLock_presult__isset {
  _Service_releaseLock_presult__isset() : expired(false), notJoined(false) {}
  bool expired;
  bool notJoined;
} _Service_releaseLock_presult__isset;

class Service_releaseLock_presult {
 public:


  virtual ~Service_releaseLock_presult() throw() {}

  LockExpired expired;
  NotJoined notJoined;

  _Service_releaseLock_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class ServiceClient : virtual public ServiceIf {
 public:
  ServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
    piprot_(prot),
    poprot_(prot) {
    iprot_ = prot.get();
    oprot_ = prot.get();
  }
  ServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) :
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
  void init(HiveState& _return);
  void send_init();
  void recv_init(HiveState& _return);
  void join(ServiceState& _return, const HostID hostID, const HiveState& hiveState);
  void send_join(const HostID hostID, const HiveState& hiveState);
  void recv_join(ServiceState& _return);
  void leave();
  void send_leave();
  void recv_leave();
  void getHiveState(HiveState& _return);
  void send_getHiveState();
  void recv_getHiveState(HiveState& _return);
  void getState(ServiceState& _return);
  void send_getState();
  void recv_getState(ServiceState& _return);
  LockID acquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout);
  void send_acquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout);
  LockID recv_acquireLock();
  void keepLock(const LockID lockID);
  void send_keepLock(const LockID lockID);
  void recv_keepLock();
  void escalateLock(const LockID lockID, const LockTimeout timeout);
  void send_escalateLock(const LockID lockID, const LockTimeout timeout);
  void recv_escalateLock();
  void releaseLock(const LockID lockID);
  void send_releaseLock(const LockID lockID);
  void recv_releaseLock();
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class ServiceProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<ServiceIf> iface_;
  virtual bool dispatchCall(apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (ServiceProcessor::*ProcessFunction)(int32_t, apache::thrift::protocol::TProtocol*, apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_init(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_join(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_leave(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_getHiveState(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_getState(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_acquireLock(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_keepLock(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_escalateLock(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_releaseLock(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  ServiceProcessor(boost::shared_ptr<ServiceIf> iface) :
    iface_(iface) {
    processMap_["init"] = &ServiceProcessor::process_init;
    processMap_["join"] = &ServiceProcessor::process_join;
    processMap_["leave"] = &ServiceProcessor::process_leave;
    processMap_["getHiveState"] = &ServiceProcessor::process_getHiveState;
    processMap_["getState"] = &ServiceProcessor::process_getState;
    processMap_["acquireLock"] = &ServiceProcessor::process_acquireLock;
    processMap_["keepLock"] = &ServiceProcessor::process_keepLock;
    processMap_["escalateLock"] = &ServiceProcessor::process_escalateLock;
    processMap_["releaseLock"] = &ServiceProcessor::process_releaseLock;
  }

  virtual ~ServiceProcessor() {}
};

class ServiceProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  ServiceProcessorFactory(const ::boost::shared_ptr< ServiceIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< ServiceIfFactory > handlerFactory_;
};

class ServiceMultiface : virtual public ServiceIf {
 public:
  ServiceMultiface(std::vector<boost::shared_ptr<ServiceIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~ServiceMultiface() {}
 protected:
  std::vector<boost::shared_ptr<ServiceIf> > ifaces_;
  ServiceMultiface() {}
  void add(boost::shared_ptr<ServiceIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void init(HiveState& _return) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->init(_return);
    }
    ifaces_[i]->init(_return);
    return;
  }

  void join(ServiceState& _return, const HostID hostID, const HiveState& hiveState) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->join(_return, hostID, hiveState);
    }
    ifaces_[i]->join(_return, hostID, hiveState);
    return;
  }

  void leave() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->leave();
    }
    ifaces_[i]->leave();
  }

  void getHiveState(HiveState& _return) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->getHiveState(_return);
    }
    ifaces_[i]->getHiveState(_return);
    return;
  }

  void getState(ServiceState& _return) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->getState(_return);
    }
    ifaces_[i]->getState(_return);
    return;
  }

  LockID acquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->acquireLock(name, mode, timeout);
    }
    return ifaces_[i]->acquireLock(name, mode, timeout);
  }

  void keepLock(const LockID lockID) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->keepLock(lockID);
    }
    ifaces_[i]->keepLock(lockID);
  }

  void escalateLock(const LockID lockID, const LockTimeout timeout) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->escalateLock(lockID, timeout);
    }
    ifaces_[i]->escalateLock(lockID, timeout);
  }

  void releaseLock(const LockID lockID) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->releaseLock(lockID);
    }
    ifaces_[i]->releaseLock(lockID);
  }

};

} // namespace

#endif
