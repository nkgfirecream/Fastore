/**
 * Autogenerated by Thrift Compiler (0.9.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef Comm_TYPES_H
#define Comm_TYPES_H

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>



namespace fastore { namespace communication {

struct RepositoryStatus {
  enum type {
    Loading = 1,
    Unloading = 2,
    Online = 3,
    Checkpointing = 4,
    Offline = 5
  };
};

extern const std::map<int, const char*> _RepositoryStatus_VALUES_TO_NAMES;

struct ServiceStatus {
  enum type {
    Unknown = 1,
    Offline = 2,
    Online = 3,
    Unreachable = 4
  };
};

extern const std::map<int, const char*> _ServiceStatus_VALUES_TO_NAMES;

struct LockMode {
  enum type {
    Read = 1,
    Write = 2
  };
};

extern const std::map<int, const char*> _LockMode_VALUES_TO_NAMES;

typedef int32_t TopologyID;

typedef int32_t ColumnID;

typedef int32_t HostID;

typedef int32_t PodID;

typedef int64_t TimeStamp;

typedef std::vector<ColumnID>  ColumnIDs;

typedef std::map<PodID, ColumnIDs>  Pods;

typedef std::map<HostID, class NetworkAddress>  HostAddresses;

typedef int64_t LockID;

typedef std::string LockName;

typedef int32_t LockTimeout;

typedef int64_t Revision;

typedef std::map<ColumnID, class ColumnWrites>  Writes;

typedef std::vector<class ValueRows>  ValueRowsList;

typedef std::map<ColumnID, class Query>  Queries;

typedef std::map<ColumnID, class ReadResult>  ReadResults;

typedef std::map<class Query, class Answer>  Read;

typedef std::map<ColumnID, Read>  Reads;

typedef struct _NetworkAddress__isset {
  _NetworkAddress__isset() : port(false) {}
  bool port;
} _NetworkAddress__isset;

class NetworkAddress {
 public:

  static const char* ascii_fingerprint; // = "18B162B1D15D8D46509D3911A9F1C2AA";
  static const uint8_t binary_fingerprint[16]; // = {0x18,0xB1,0x62,0xB1,0xD1,0x5D,0x8D,0x46,0x50,0x9D,0x39,0x11,0xA9,0xF1,0xC2,0xAA};

  NetworkAddress() : name(), port(0) {
  }

  virtual ~NetworkAddress() throw() {}

  std::string name;
  int32_t port;

  _NetworkAddress__isset __isset;

  void __set_name(const std::string& val) {
    name = val;
  }

  void __set_port(const int32_t val) {
    port = val;
    __isset.port = true;
  }

  bool operator == (const NetworkAddress & rhs) const
  {
    if (!(name == rhs.name))
      return false;
    if (__isset.port != rhs.__isset.port)
      return false;
    else if (__isset.port && !(port == rhs.port))
      return false;
    return true;
  }
  bool operator != (const NetworkAddress &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const NetworkAddress & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(NetworkAddress &a, NetworkAddress &b);


class WorkerState {
 public:

  static const char* ascii_fingerprint; // = "FD84F91A115D56581604124C0B6B3D24";
  static const uint8_t binary_fingerprint[16]; // = {0xFD,0x84,0xF9,0x1A,0x11,0x5D,0x56,0x58,0x16,0x04,0x12,0x4C,0x0B,0x6B,0x3D,0x24};

  WorkerState() : podID(0), port(0) {
  }

  virtual ~WorkerState() throw() {}

  PodID podID;
  std::map<ColumnID, RepositoryStatus::type>  repositoryStatus;
  int32_t port;

  void __set_podID(const PodID val) {
    podID = val;
  }

  void __set_repositoryStatus(const std::map<ColumnID, RepositoryStatus::type> & val) {
    repositoryStatus = val;
  }

  void __set_port(const int32_t val) {
    port = val;
  }

  bool operator == (const WorkerState & rhs) const
  {
    if (!(podID == rhs.podID))
      return false;
    if (!(repositoryStatus == rhs.repositoryStatus))
      return false;
    if (!(port == rhs.port))
      return false;
    return true;
  }
  bool operator != (const WorkerState &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const WorkerState & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(WorkerState &a, WorkerState &b);


class ServiceState {
 public:

  static const char* ascii_fingerprint; // = "AE98C326F1D501E0750917B29756621D";
  static const uint8_t binary_fingerprint[16]; // = {0xAE,0x98,0xC3,0x26,0xF1,0xD5,0x01,0xE0,0x75,0x09,0x17,0xB2,0x97,0x56,0x62,0x1D};

  ServiceState() : status((ServiceStatus::type)0), timeStamp(0) {
  }

  virtual ~ServiceState() throw() {}

  ServiceStatus::type status;
  TimeStamp timeStamp;
  NetworkAddress address;
  std::vector<WorkerState>  workers;

  void __set_status(const ServiceStatus::type val) {
    status = val;
  }

  void __set_timeStamp(const TimeStamp val) {
    timeStamp = val;
  }

  void __set_address(const NetworkAddress& val) {
    address = val;
  }

  void __set_workers(const std::vector<WorkerState> & val) {
    workers = val;
  }

  bool operator == (const ServiceState & rhs) const
  {
    if (!(status == rhs.status))
      return false;
    if (!(timeStamp == rhs.timeStamp))
      return false;
    if (!(address == rhs.address))
      return false;
    if (!(workers == rhs.workers))
      return false;
    return true;
  }
  bool operator != (const ServiceState &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ServiceState & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(ServiceState &a, ServiceState &b);


class HiveState {
 public:

  static const char* ascii_fingerprint; // = "E5BEEF2EC9A3602FBD778958F04C31BA";
  static const uint8_t binary_fingerprint[16]; // = {0xE5,0xBE,0xEF,0x2E,0xC9,0xA3,0x60,0x2F,0xBD,0x77,0x89,0x58,0xF0,0x4C,0x31,0xBA};

  HiveState() : topologyID(0), reportingHostID(0) {
  }

  virtual ~HiveState() throw() {}

  TopologyID topologyID;
  std::map<HostID, ServiceState>  services;
  HostID reportingHostID;

  void __set_topologyID(const TopologyID val) {
    topologyID = val;
  }

  void __set_services(const std::map<HostID, ServiceState> & val) {
    services = val;
  }

  void __set_reportingHostID(const HostID val) {
    reportingHostID = val;
  }

  bool operator == (const HiveState & rhs) const
  {
    if (!(topologyID == rhs.topologyID))
      return false;
    if (!(services == rhs.services))
      return false;
    if (!(reportingHostID == rhs.reportingHostID))
      return false;
    return true;
  }
  bool operator != (const HiveState &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const HiveState & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(HiveState &a, HiveState &b);


class Topology {
 public:

  static const char* ascii_fingerprint; // = "8FFF1DF7A12AE69FE78268BC347A1EAE";
  static const uint8_t binary_fingerprint[16]; // = {0x8F,0xFF,0x1D,0xF7,0xA1,0x2A,0xE6,0x9F,0xE7,0x82,0x68,0xBC,0x34,0x7A,0x1E,0xAE};

  Topology() : topologyID(0) {
  }

  virtual ~Topology() throw() {}

  TopologyID topologyID;
  std::map<HostID, Pods>  hosts;

  void __set_topologyID(const TopologyID val) {
    topologyID = val;
  }

  void __set_hosts(const std::map<HostID, Pods> & val) {
    hosts = val;
  }

  bool operator == (const Topology & rhs) const
  {
    if (!(topologyID == rhs.topologyID))
      return false;
    if (!(hosts == rhs.hosts))
      return false;
    return true;
  }
  bool operator != (const Topology &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Topology & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Topology &a, Topology &b);

typedef struct _OptionalHiveState__isset {
  _OptionalHiveState__isset() : hiveState(false), potentialWorkers(false) {}
  bool hiveState;
  bool potentialWorkers;
} _OptionalHiveState__isset;

class OptionalHiveState {
 public:

  static const char* ascii_fingerprint; // = "63624E84E96F444A04F95217FD47D34B";
  static const uint8_t binary_fingerprint[16]; // = {0x63,0x62,0x4E,0x84,0xE9,0x6F,0x44,0x4A,0x04,0xF9,0x52,0x17,0xFD,0x47,0xD3,0x4B};

  OptionalHiveState() : potentialWorkers(0) {
  }

  virtual ~OptionalHiveState() throw() {}

  HiveState hiveState;
  int32_t potentialWorkers;

  _OptionalHiveState__isset __isset;

  void __set_hiveState(const HiveState& val) {
    hiveState = val;
    __isset.hiveState = true;
  }

  void __set_potentialWorkers(const int32_t val) {
    potentialWorkers = val;
    __isset.potentialWorkers = true;
  }

  bool operator == (const OptionalHiveState & rhs) const
  {
    if (__isset.hiveState != rhs.__isset.hiveState)
      return false;
    else if (__isset.hiveState && !(hiveState == rhs.hiveState))
      return false;
    if (__isset.potentialWorkers != rhs.__isset.potentialWorkers)
      return false;
    else if (__isset.potentialWorkers && !(potentialWorkers == rhs.potentialWorkers))
      return false;
    return true;
  }
  bool operator != (const OptionalHiveState &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const OptionalHiveState & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(OptionalHiveState &a, OptionalHiveState &b);

typedef struct _OptionalServiceState__isset {
  _OptionalServiceState__isset() : serviceState(false), potentialWorkers(false) {}
  bool serviceState;
  bool potentialWorkers;
} _OptionalServiceState__isset;

class OptionalServiceState {
 public:

  static const char* ascii_fingerprint; // = "F1C97296E97AFFC0B889BA7859100316";
  static const uint8_t binary_fingerprint[16]; // = {0xF1,0xC9,0x72,0x96,0xE9,0x7A,0xFF,0xC0,0xB8,0x89,0xBA,0x78,0x59,0x10,0x03,0x16};

  OptionalServiceState() : potentialWorkers(0) {
  }

  virtual ~OptionalServiceState() throw() {}

  ServiceState serviceState;
  int32_t potentialWorkers;

  _OptionalServiceState__isset __isset;

  void __set_serviceState(const ServiceState& val) {
    serviceState = val;
    __isset.serviceState = true;
  }

  void __set_potentialWorkers(const int32_t val) {
    potentialWorkers = val;
    __isset.potentialWorkers = true;
  }

  bool operator == (const OptionalServiceState & rhs) const
  {
    if (__isset.serviceState != rhs.__isset.serviceState)
      return false;
    else if (__isset.serviceState && !(serviceState == rhs.serviceState))
      return false;
    if (__isset.potentialWorkers != rhs.__isset.potentialWorkers)
      return false;
    else if (__isset.potentialWorkers && !(potentialWorkers == rhs.potentialWorkers))
      return false;
    return true;
  }
  bool operator != (const OptionalServiceState &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const OptionalServiceState & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(OptionalServiceState &a, OptionalServiceState &b);

typedef struct _LockExpired__isset {
  _LockExpired__isset() : lockID(false) {}
  bool lockID;
} _LockExpired__isset;

class LockExpired : public ::apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "56A59CE7FFAF82BCA8A19FAACDE4FB75";
  static const uint8_t binary_fingerprint[16]; // = {0x56,0xA5,0x9C,0xE7,0xFF,0xAF,0x82,0xBC,0xA8,0xA1,0x9F,0xAA,0xCD,0xE4,0xFB,0x75};

  LockExpired() : lockID(0) {
  }

  virtual ~LockExpired() throw() {}

  LockID lockID;

  _LockExpired__isset __isset;

  void __set_lockID(const LockID val) {
    lockID = val;
  }

  bool operator == (const LockExpired & rhs) const
  {
    if (!(lockID == rhs.lockID))
      return false;
    return true;
  }
  bool operator != (const LockExpired &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const LockExpired & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(LockExpired &a, LockExpired &b);


class LockTimedOut : public ::apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};

  LockTimedOut() {
  }

  virtual ~LockTimedOut() throw() {}


  bool operator == (const LockTimedOut & /* rhs */) const
  {
    return true;
  }
  bool operator != (const LockTimedOut &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const LockTimedOut & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(LockTimedOut &a, LockTimedOut &b);

typedef struct _AlreadyJoined__isset {
  _AlreadyJoined__isset() : hostID(false) {}
  bool hostID;
} _AlreadyJoined__isset;

class AlreadyJoined : public ::apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "E86CACEB22240450EDCBEFC3A83970E4";
  static const uint8_t binary_fingerprint[16]; // = {0xE8,0x6C,0xAC,0xEB,0x22,0x24,0x04,0x50,0xED,0xCB,0xEF,0xC3,0xA8,0x39,0x70,0xE4};

  AlreadyJoined() : hostID(0) {
  }

  virtual ~AlreadyJoined() throw() {}

  HostID hostID;

  _AlreadyJoined__isset __isset;

  void __set_hostID(const HostID val) {
    hostID = val;
  }

  bool operator == (const AlreadyJoined & rhs) const
  {
    if (!(hostID == rhs.hostID))
      return false;
    return true;
  }
  bool operator != (const AlreadyJoined &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const AlreadyJoined & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(AlreadyJoined &a, AlreadyJoined &b);

typedef struct _NotJoined__isset {
  _NotJoined__isset() : potentialWorkers(false) {}
  bool potentialWorkers;
} _NotJoined__isset;

class NotJoined : public ::apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "E86CACEB22240450EDCBEFC3A83970E4";
  static const uint8_t binary_fingerprint[16]; // = {0xE8,0x6C,0xAC,0xEB,0x22,0x24,0x04,0x50,0xED,0xCB,0xEF,0xC3,0xA8,0x39,0x70,0xE4};

  NotJoined() : potentialWorkers(0) {
  }

  virtual ~NotJoined() throw() {}

  int32_t potentialWorkers;

  _NotJoined__isset __isset;

  void __set_potentialWorkers(const int32_t val) {
    potentialWorkers = val;
  }

  bool operator == (const NotJoined & rhs) const
  {
    if (!(potentialWorkers == rhs.potentialWorkers))
      return false;
    return true;
  }
  bool operator != (const NotJoined &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const NotJoined & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(NotJoined &a, NotJoined &b);


class TransactionID {
 public:

  static const char* ascii_fingerprint; // = "F33135321253DAEB67B0E79E416CA831";
  static const uint8_t binary_fingerprint[16]; // = {0xF3,0x31,0x35,0x32,0x12,0x53,0xDA,0xEB,0x67,0xB0,0xE7,0x9E,0x41,0x6C,0xA8,0x31};

  TransactionID() : revision(0), key(0) {
  }

  virtual ~TransactionID() throw() {}

  Revision revision;
  int64_t key;

  void __set_revision(const Revision val) {
    revision = val;
  }

  void __set_key(const int64_t val) {
    key = val;
  }

  bool operator == (const TransactionID & rhs) const
  {
    if (!(revision == rhs.revision))
      return false;
    if (!(key == rhs.key))
      return false;
    return true;
  }
  bool operator != (const TransactionID &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TransactionID & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(TransactionID &a, TransactionID &b);


class Include {
 public:

  static const char* ascii_fingerprint; // = "07A9615F837F7D0A952B595DD3020972";
  static const uint8_t binary_fingerprint[16]; // = {0x07,0xA9,0x61,0x5F,0x83,0x7F,0x7D,0x0A,0x95,0x2B,0x59,0x5D,0xD3,0x02,0x09,0x72};

  Include() : rowID(), value() {
  }

  virtual ~Include() throw() {}

  std::string rowID;
  std::string value;

  void __set_rowID(const std::string& val) {
    rowID = val;
  }

  void __set_value(const std::string& val) {
    value = val;
  }

  bool operator == (const Include & rhs) const
  {
    if (!(rowID == rhs.rowID))
      return false;
    if (!(value == rhs.value))
      return false;
    return true;
  }
  bool operator != (const Include &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Include & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Include &a, Include &b);


class Exclude {
 public:

  static const char* ascii_fingerprint; // = "EFB929595D312AC8F305D5A794CFEDA1";
  static const uint8_t binary_fingerprint[16]; // = {0xEF,0xB9,0x29,0x59,0x5D,0x31,0x2A,0xC8,0xF3,0x05,0xD5,0xA7,0x94,0xCF,0xED,0xA1};

  Exclude() : rowID() {
  }

  virtual ~Exclude() throw() {}

  std::string rowID;

  void __set_rowID(const std::string& val) {
    rowID = val;
  }

  bool operator == (const Exclude & rhs) const
  {
    if (!(rowID == rhs.rowID))
      return false;
    return true;
  }
  bool operator != (const Exclude &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Exclude & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Exclude &a, Exclude &b);

typedef struct _ColumnWrites__isset {
  _ColumnWrites__isset() : includes(false), excludes(false) {}
  bool includes;
  bool excludes;
} _ColumnWrites__isset;

class ColumnWrites {
 public:

  static const char* ascii_fingerprint; // = "7B474451502DF848AF475006A25AB746";
  static const uint8_t binary_fingerprint[16]; // = {0x7B,0x47,0x44,0x51,0x50,0x2D,0xF8,0x48,0xAF,0x47,0x50,0x06,0xA2,0x5A,0xB7,0x46};

  ColumnWrites() {
  }

  virtual ~ColumnWrites() throw() {}

  std::vector<Include>  includes;
  std::vector<Exclude>  excludes;

  _ColumnWrites__isset __isset;

  void __set_includes(const std::vector<Include> & val) {
    includes = val;
    __isset.includes = true;
  }

  void __set_excludes(const std::vector<Exclude> & val) {
    excludes = val;
    __isset.excludes = true;
  }

  bool operator == (const ColumnWrites & rhs) const
  {
    if (__isset.includes != rhs.__isset.includes)
      return false;
    else if (__isset.includes && !(includes == rhs.includes))
      return false;
    if (__isset.excludes != rhs.__isset.excludes)
      return false;
    else if (__isset.excludes && !(excludes == rhs.excludes))
      return false;
    return true;
  }
  bool operator != (const ColumnWrites &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ColumnWrites & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(ColumnWrites &a, ColumnWrites &b);


class Statistic {
 public:

  static const char* ascii_fingerprint; // = "F33135321253DAEB67B0E79E416CA831";
  static const uint8_t binary_fingerprint[16]; // = {0xF3,0x31,0x35,0x32,0x12,0x53,0xDA,0xEB,0x67,0xB0,0xE7,0x9E,0x41,0x6C,0xA8,0x31};

  Statistic() : total(0), unique(0) {
  }

  virtual ~Statistic() throw() {}

  int64_t total;
  int64_t unique;

  void __set_total(const int64_t val) {
    total = val;
  }

  void __set_unique(const int64_t val) {
    unique = val;
  }

  bool operator == (const Statistic & rhs) const
  {
    if (!(total == rhs.total))
      return false;
    if (!(unique == rhs.unique))
      return false;
    return true;
  }
  bool operator != (const Statistic &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Statistic & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Statistic &a, Statistic &b);


class RangeBound {
 public:

  static const char* ascii_fingerprint; // = "7D61C9AA00102AB4D8F72A1DA58297DC";
  static const uint8_t binary_fingerprint[16]; // = {0x7D,0x61,0xC9,0xAA,0x00,0x10,0x2A,0xB4,0xD8,0xF7,0x2A,0x1D,0xA5,0x82,0x97,0xDC};

  RangeBound() : value(), inclusive(0) {
  }

  virtual ~RangeBound() throw() {}

  std::string value;
  bool inclusive;

  void __set_value(const std::string& val) {
    value = val;
  }

  void __set_inclusive(const bool val) {
    inclusive = val;
  }

  bool operator == (const RangeBound & rhs) const
  {
    if (!(value == rhs.value))
      return false;
    if (!(inclusive == rhs.inclusive))
      return false;
    return true;
  }
  bool operator != (const RangeBound &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const RangeBound & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(RangeBound &a, RangeBound &b);

typedef struct _RangeRequest__isset {
  _RangeRequest__isset() : first(false), last(false), rowID(false) {}
  bool first;
  bool last;
  bool rowID;
} _RangeRequest__isset;

class RangeRequest {
 public:

  static const char* ascii_fingerprint; // = "4C1A8C7B2474BEEF3B5899689B4AC289";
  static const uint8_t binary_fingerprint[16]; // = {0x4C,0x1A,0x8C,0x7B,0x24,0x74,0xBE,0xEF,0x3B,0x58,0x99,0x68,0x9B,0x4A,0xC2,0x89};

  RangeRequest() : ascending(true), limit(500), rowID() {
  }

  virtual ~RangeRequest() throw() {}

  bool ascending;
  int32_t limit;
  RangeBound first;
  RangeBound last;
  std::string rowID;

  _RangeRequest__isset __isset;

  void __set_ascending(const bool val) {
    ascending = val;
  }

  void __set_limit(const int32_t val) {
    limit = val;
  }

  void __set_first(const RangeBound& val) {
    first = val;
    __isset.first = true;
  }

  void __set_last(const RangeBound& val) {
    last = val;
    __isset.last = true;
  }

  void __set_rowID(const std::string& val) {
    rowID = val;
    __isset.rowID = true;
  }

  bool operator == (const RangeRequest & rhs) const
  {
    if (!(ascending == rhs.ascending))
      return false;
    if (!(limit == rhs.limit))
      return false;
    if (__isset.first != rhs.__isset.first)
      return false;
    else if (__isset.first && !(first == rhs.first))
      return false;
    if (__isset.last != rhs.__isset.last)
      return false;
    else if (__isset.last && !(last == rhs.last))
      return false;
    if (__isset.rowID != rhs.__isset.rowID)
      return false;
    else if (__isset.rowID && !(rowID == rhs.rowID))
      return false;
    return true;
  }
  bool operator != (const RangeRequest &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const RangeRequest & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(RangeRequest &a, RangeRequest &b);


class ValueRows {
 public:

  static const char* ascii_fingerprint; // = "25702B8D5E28AA39160F267DABBC8446";
  static const uint8_t binary_fingerprint[16]; // = {0x25,0x70,0x2B,0x8D,0x5E,0x28,0xAA,0x39,0x16,0x0F,0x26,0x7D,0xAB,0xBC,0x84,0x46};

  ValueRows() : value() {
  }

  virtual ~ValueRows() throw() {}

  std::string value;
  std::vector<std::string>  rowIDs;

  void __set_value(const std::string& val) {
    value = val;
  }

  void __set_rowIDs(const std::vector<std::string> & val) {
    rowIDs = val;
  }

  bool operator == (const ValueRows & rhs) const
  {
    if (!(value == rhs.value))
      return false;
    if (!(rowIDs == rhs.rowIDs))
      return false;
    return true;
  }
  bool operator != (const ValueRows &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ValueRows & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(ValueRows &a, ValueRows &b);


class RangeResult {
 public:

  static const char* ascii_fingerprint; // = "A6BFD4A548133149EF24E6ED4F1025B8";
  static const uint8_t binary_fingerprint[16]; // = {0xA6,0xBF,0xD4,0xA5,0x48,0x13,0x31,0x49,0xEF,0x24,0xE6,0xED,0x4F,0x10,0x25,0xB8};

  RangeResult() : eof(0), bof(0), limited(0) {
  }

  virtual ~RangeResult() throw() {}

  ValueRowsList valueRowsList;
  bool eof;
  bool bof;
  bool limited;

  void __set_valueRowsList(const ValueRowsList& val) {
    valueRowsList = val;
  }

  void __set_eof(const bool val) {
    eof = val;
  }

  void __set_bof(const bool val) {
    bof = val;
  }

  void __set_limited(const bool val) {
    limited = val;
  }

  bool operator == (const RangeResult & rhs) const
  {
    if (!(valueRowsList == rhs.valueRowsList))
      return false;
    if (!(eof == rhs.eof))
      return false;
    if (!(bof == rhs.bof))
      return false;
    if (!(limited == rhs.limited))
      return false;
    return true;
  }
  bool operator != (const RangeResult &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const RangeResult & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(RangeResult &a, RangeResult &b);

typedef struct _Query__isset {
  _Query__isset() : rowIDs(false), ranges(false) {}
  bool rowIDs;
  bool ranges;
} _Query__isset;

class Query {
 public:

  static const char* ascii_fingerprint; // = "C06DB9A662129FF3CAE53510F3482EB5";
  static const uint8_t binary_fingerprint[16]; // = {0xC0,0x6D,0xB9,0xA6,0x62,0x12,0x9F,0xF3,0xCA,0xE5,0x35,0x10,0xF3,0x48,0x2E,0xB5};

  Query() {
  }

  virtual ~Query() throw() {}

  std::vector<std::string>  rowIDs;
  std::vector<RangeRequest>  ranges;

  _Query__isset __isset;

  void __set_rowIDs(const std::vector<std::string> & val) {
    rowIDs = val;
    __isset.rowIDs = true;
  }

  void __set_ranges(const std::vector<RangeRequest> & val) {
    ranges = val;
    __isset.ranges = true;
  }

  bool operator == (const Query & rhs) const
  {
    if (__isset.rowIDs != rhs.__isset.rowIDs)
      return false;
    else if (__isset.rowIDs && !(rowIDs == rhs.rowIDs))
      return false;
    if (__isset.ranges != rhs.__isset.ranges)
      return false;
    else if (__isset.ranges && !(ranges == rhs.ranges))
      return false;
    return true;
  }
  bool operator != (const Query &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Query & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Query &a, Query &b);

typedef struct _Answer__isset {
  _Answer__isset() : rowIDValues(false), rangeValues(false) {}
  bool rowIDValues;
  bool rangeValues;
} _Answer__isset;

class Answer {
 public:

  static const char* ascii_fingerprint; // = "2AE4F329512B9BFF283AD9D6279E124D";
  static const uint8_t binary_fingerprint[16]; // = {0x2A,0xE4,0xF3,0x29,0x51,0x2B,0x9B,0xFF,0x28,0x3A,0xD9,0xD6,0x27,0x9E,0x12,0x4D};

  Answer() {
  }

  virtual ~Answer() throw() {}

  std::vector<std::string>  rowIDValues;
  std::vector<RangeResult>  rangeValues;

  _Answer__isset __isset;

  void __set_rowIDValues(const std::vector<std::string> & val) {
    rowIDValues = val;
    __isset.rowIDValues = true;
  }

  void __set_rangeValues(const std::vector<RangeResult> & val) {
    rangeValues = val;
    __isset.rangeValues = true;
  }

  bool operator == (const Answer & rhs) const
  {
    if (__isset.rowIDValues != rhs.__isset.rowIDValues)
      return false;
    else if (__isset.rowIDValues && !(rowIDValues == rhs.rowIDValues))
      return false;
    if (__isset.rangeValues != rhs.__isset.rangeValues)
      return false;
    else if (__isset.rangeValues && !(rangeValues == rhs.rangeValues))
      return false;
    return true;
  }
  bool operator != (const Answer &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Answer & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Answer &a, Answer &b);


class ReadResult {
 public:

  static const char* ascii_fingerprint; // = "747FE1FFF63CCB61ABEB89C586E23870";
  static const uint8_t binary_fingerprint[16]; // = {0x74,0x7F,0xE1,0xFF,0xF6,0x3C,0xCB,0x61,0xAB,0xEB,0x89,0xC5,0x86,0xE2,0x38,0x70};

  ReadResult() : revision(0) {
  }

  virtual ~ReadResult() throw() {}

  Answer answer;
  Revision revision;

  void __set_answer(const Answer& val) {
    answer = val;
  }

  void __set_revision(const Revision val) {
    revision = val;
  }

  bool operator == (const ReadResult & rhs) const
  {
    if (!(answer == rhs.answer))
      return false;
    if (!(revision == rhs.revision))
      return false;
    return true;
  }
  bool operator != (const ReadResult &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ReadResult & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(ReadResult &a, ReadResult &b);

typedef struct _NotLatest__isset {
  _NotLatest__isset() : latest(false) {}
  bool latest;
} _NotLatest__isset;

class NotLatest : public ::apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "56A59CE7FFAF82BCA8A19FAACDE4FB75";
  static const uint8_t binary_fingerprint[16]; // = {0x56,0xA5,0x9C,0xE7,0xFF,0xAF,0x82,0xBC,0xA8,0xA1,0x9F,0xAA,0xCD,0xE4,0xFB,0x75};

  NotLatest() : latest(0) {
  }

  virtual ~NotLatest() throw() {}

  Revision latest;

  _NotLatest__isset __isset;

  void __set_latest(const Revision val) {
    latest = val;
  }

  bool operator == (const NotLatest & rhs) const
  {
    if (!(latest == rhs.latest))
      return false;
    return true;
  }
  bool operator != (const NotLatest &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const NotLatest & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(NotLatest &a, NotLatest &b);

typedef struct _AlreadyPending__isset {
  _AlreadyPending__isset() : pendingTransactionID(false) {}
  bool pendingTransactionID;
} _AlreadyPending__isset;

class AlreadyPending : public ::apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "9CFE4A6581B5B8EB11F5BBBCEFA07940";
  static const uint8_t binary_fingerprint[16]; // = {0x9C,0xFE,0x4A,0x65,0x81,0xB5,0xB8,0xEB,0x11,0xF5,0xBB,0xBC,0xEF,0xA0,0x79,0x40};

  AlreadyPending() {
  }

  virtual ~AlreadyPending() throw() {}

  TransactionID pendingTransactionID;

  _AlreadyPending__isset __isset;

  void __set_pendingTransactionID(const TransactionID& val) {
    pendingTransactionID = val;
  }

  bool operator == (const AlreadyPending & rhs) const
  {
    if (!(pendingTransactionID == rhs.pendingTransactionID))
      return false;
    return true;
  }
  bool operator != (const AlreadyPending &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const AlreadyPending & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(AlreadyPending &a, AlreadyPending &b);

typedef struct _Conflict__isset {
  _Conflict__isset() : details(false), columnIDs(false) {}
  bool details;
  bool columnIDs;
} _Conflict__isset;

class Conflict : public ::apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "920F6571EE6C0CF61556A788D6042213";
  static const uint8_t binary_fingerprint[16]; // = {0x92,0x0F,0x65,0x71,0xEE,0x6C,0x0C,0xF6,0x15,0x56,0xA7,0x88,0xD6,0x04,0x22,0x13};

  Conflict() : details() {
  }

  virtual ~Conflict() throw() {}

  std::string details;
  std::vector<ColumnID>  columnIDs;

  _Conflict__isset __isset;

  void __set_details(const std::string& val) {
    details = val;
  }

  void __set_columnIDs(const std::vector<ColumnID> & val) {
    columnIDs = val;
  }

  bool operator == (const Conflict & rhs) const
  {
    if (!(details == rhs.details))
      return false;
    if (!(columnIDs == rhs.columnIDs))
      return false;
    return true;
  }
  bool operator != (const Conflict &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Conflict & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Conflict &a, Conflict &b);

typedef struct _BeyondHistory__isset {
  _BeyondHistory__isset() : minHistory(false) {}
  bool minHistory;
} _BeyondHistory__isset;

class BeyondHistory : public ::apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "56A59CE7FFAF82BCA8A19FAACDE4FB75";
  static const uint8_t binary_fingerprint[16]; // = {0x56,0xA5,0x9C,0xE7,0xFF,0xAF,0x82,0xBC,0xA8,0xA1,0x9F,0xAA,0xCD,0xE4,0xFB,0x75};

  BeyondHistory() : minHistory(0) {
  }

  virtual ~BeyondHistory() throw() {}

  Revision minHistory;

  _BeyondHistory__isset __isset;

  void __set_minHistory(const Revision val) {
    minHistory = val;
  }

  bool operator == (const BeyondHistory & rhs) const
  {
    if (!(minHistory == rhs.minHistory))
      return false;
    return true;
  }
  bool operator != (const BeyondHistory &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const BeyondHistory & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(BeyondHistory &a, BeyondHistory &b);

}} // namespace

#endif
