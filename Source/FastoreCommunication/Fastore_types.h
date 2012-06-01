/**
 * Autogenerated by Thrift Compiler (0.9.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef Fastore_TYPES_H
#define Fastore_TYPES_H

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>



namespace fastore {

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
    Offline = 1,
    Online = 2,
    Unreachable = 3
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

typedef int64_t Revision;

typedef int32_t TopologyID;

typedef int32_t ColumnID;

typedef int32_t HostID;

typedef int32_t PodID;

typedef std::string NetworkAddress;

typedef int32_t NetworkPort;

typedef int64_t TimeStamp;

typedef int64_t LockID;

typedef std::string LockName;

typedef int32_t LockTimeout;

typedef std::map<ColumnID, class ColumnWrites>  Writes;

typedef std::vector<class ValueRows>  ValueRowsList;

typedef std::map<ColumnID, class Query>  Queries;

typedef std::map<ColumnID, class ReadResult>  ReadResults;

typedef std::map<class Query, class Answer>  Read;

typedef std::map<ColumnID, Read>  Reads;

typedef struct _Pod__isset {
  _Pod__isset() : columnIDs(false) {}
  bool columnIDs;
} _Pod__isset;

class Pod {
 public:

  static const char* ascii_fingerprint; // = "FF7335CAA8E1AFD6418DDE8FC093C053";
  static const uint8_t binary_fingerprint[16]; // = {0xFF,0x73,0x35,0xCA,0xA8,0xE1,0xAF,0xD6,0x41,0x8D,0xDE,0x8F,0xC0,0x93,0xC0,0x53};

  Pod() {
  }

  virtual ~Pod() throw() {}

  std::set<ColumnID>  columnIDs;

  _Pod__isset __isset;

  void __set_columnIDs(const std::set<ColumnID> & val) {
    columnIDs = val;
  }

  bool operator == (const Pod & rhs) const
  {
    if (!(columnIDs == rhs.columnIDs))
      return false;
    return true;
  }
  bool operator != (const Pod &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Pod & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Pod &a, Pod &b);


class Host {
 public:

  static const char* ascii_fingerprint; // = "7D03D12D882B7BC9C0B19273F944DDBF";
  static const uint8_t binary_fingerprint[16]; // = {0x7D,0x03,0xD1,0x2D,0x88,0x2B,0x7B,0xC9,0xC0,0xB1,0x92,0x73,0xF9,0x44,0xDD,0xBF};

  Host() {
  }

  virtual ~Host() throw() {}

  std::map<PodID, Pod>  pods;

  void __set_pods(const std::map<PodID, Pod> & val) {
    pods = val;
  }

  bool operator == (const Host & rhs) const
  {
    if (!(pods == rhs.pods))
      return false;
    return true;
  }
  bool operator != (const Host &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Host & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Host &a, Host &b);


class Topology {
 public:

  static const char* ascii_fingerprint; // = "D03993086D93441815A7DE971F647F31";
  static const uint8_t binary_fingerprint[16]; // = {0xD0,0x39,0x93,0x08,0x6D,0x93,0x44,0x18,0x15,0xA7,0xDE,0x97,0x1F,0x64,0x7F,0x31};

  Topology() : id(0) {
  }

  virtual ~Topology() throw() {}

  TopologyID id;
  std::map<HostID, Host>  hosts;

  void __set_id(const TopologyID val) {
    id = val;
  }

  void __set_hosts(const std::map<HostID, Host> & val) {
    hosts = val;
  }

  bool operator == (const Topology & rhs) const
  {
    if (!(id == rhs.id))
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


class TopologyResult {
 public:

  static const char* ascii_fingerprint; // = "B2F811563D061F49A14BDC842D3A5102";
  static const uint8_t binary_fingerprint[16]; // = {0xB2,0xF8,0x11,0x56,0x3D,0x06,0x1F,0x49,0xA1,0x4B,0xDC,0x84,0x2D,0x3A,0x51,0x02};

  TopologyResult() : revision(0) {
  }

  virtual ~TopologyResult() throw() {}

  Topology topology;
  Revision revision;

  void __set_topology(const Topology& val) {
    topology = val;
  }

  void __set_revision(const Revision val) {
    revision = val;
  }

  bool operator == (const TopologyResult & rhs) const
  {
    if (!(topology == rhs.topology))
      return false;
    if (!(revision == rhs.revision))
      return false;
    return true;
  }
  bool operator != (const TopologyResult &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TopologyResult & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(TopologyResult &a, TopologyResult &b);


class WorkerState {
 public:

  static const char* ascii_fingerprint; // = "D576A5D4D687D9BDB02F6FC8D2506D9E";
  static const uint8_t binary_fingerprint[16]; // = {0xD5,0x76,0xA5,0xD4,0xD6,0x87,0xD9,0xBD,0xB0,0x2F,0x6F,0xC8,0xD2,0x50,0x6D,0x9E};

  WorkerState() : port(0) {
  }

  virtual ~WorkerState() throw() {}

  std::map<ColumnID, RepositoryStatus::type>  repositoryStatus;
  NetworkPort port;

  void __set_repositoryStatus(const std::map<ColumnID, RepositoryStatus::type> & val) {
    repositoryStatus = val;
  }

  void __set_port(const NetworkPort val) {
    port = val;
  }

  bool operator == (const WorkerState & rhs) const
  {
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

typedef struct _ServiceState__isset {
  _ServiceState__isset() : port(false) {}
  bool port;
} _ServiceState__isset;

class ServiceState {
 public:

  static const char* ascii_fingerprint; // = "38FDE7751FA147BC46C0BC954603E9EE";
  static const uint8_t binary_fingerprint[16]; // = {0x38,0xFD,0xE7,0x75,0x1F,0xA1,0x47,0xBC,0x46,0xC0,0xBC,0x95,0x46,0x03,0xE9,0xEE};

  ServiceState() : status((ServiceStatus::type)0), timeStamp(0), address(), port(0) {
  }

  virtual ~ServiceState() throw() {}

  ServiceStatus::type status;
  TimeStamp timeStamp;
  NetworkAddress address;
  NetworkPort port;
  std::map<PodID, WorkerState>  workers;

  _ServiceState__isset __isset;

  void __set_status(const ServiceStatus::type val) {
    status = val;
  }

  void __set_timeStamp(const TimeStamp val) {
    timeStamp = val;
  }

  void __set_address(const NetworkAddress& val) {
    address = val;
  }

  void __set_port(const NetworkPort val) {
    port = val;
    __isset.port = true;
  }

  void __set_workers(const std::map<PodID, WorkerState> & val) {
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
    if (__isset.port != rhs.__isset.port)
      return false;
    else if (__isset.port && !(port == rhs.port))
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

  static const char* ascii_fingerprint; // = "1E74F1F34021F1762D541650812428EA";
  static const uint8_t binary_fingerprint[16]; // = {0x1E,0x74,0xF1,0xF3,0x40,0x21,0xF1,0x76,0x2D,0x54,0x16,0x50,0x81,0x24,0x28,0xEA};

  HiveState() : topologyID(0), hostID(0) {
  }

  virtual ~HiveState() throw() {}

  TopologyID topologyID;
  std::map<HostID, ServiceState>  services;
  HostID hostID;

  void __set_topologyID(const TopologyID val) {
    topologyID = val;
  }

  void __set_services(const std::map<HostID, ServiceState> & val) {
    services = val;
  }

  void __set_hostID(const HostID val) {
    hostID = val;
  }

  bool operator == (const HiveState & rhs) const
  {
    if (!(topologyID == rhs.topologyID))
      return false;
    if (!(services == rhs.services))
      return false;
    if (!(hostID == rhs.hostID))
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

  static const char* ascii_fingerprint; // = "ED99C79CAEE6175252083848E86F96EC";
  static const uint8_t binary_fingerprint[16]; // = {0xED,0x99,0xC7,0x9C,0xAE,0xE6,0x17,0x52,0x52,0x08,0x38,0x48,0xE8,0x6F,0x96,0xEC};

  RangeRequest() : ascending(true), rowID() {
  }

  virtual ~RangeRequest() throw() {}

  bool ascending;
  RangeBound first;
  RangeBound last;
  std::string rowID;

  _RangeRequest__isset __isset;

  void __set_ascending(const bool val) {
    ascending = val;
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

  RangeResult() : endOfRange(0), beginOfRange(0), limited(0) {
  }

  virtual ~RangeResult() throw() {}

  ValueRowsList valueRowsList;
  bool endOfRange;
  bool beginOfRange;
  bool limited;

  void __set_valueRowsList(const ValueRowsList& val) {
    valueRowsList = val;
  }

  void __set_endOfRange(const bool val) {
    endOfRange = val;
  }

  void __set_beginOfRange(const bool val) {
    beginOfRange = val;
  }

  void __set_limited(const bool val) {
    limited = val;
  }

  bool operator == (const RangeResult & rhs) const
  {
    if (!(valueRowsList == rhs.valueRowsList))
      return false;
    if (!(endOfRange == rhs.endOfRange))
      return false;
    if (!(beginOfRange == rhs.beginOfRange))
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

  static const char* ascii_fingerprint; // = "53486A2100C5B12C0F6DC8B42E976024";
  static const uint8_t binary_fingerprint[16]; // = {0x53,0x48,0x6A,0x21,0x00,0xC5,0xB1,0x2C,0x0F,0x6D,0xC8,0xB4,0x2E,0x97,0x60,0x24};

  Query() : limit(500) {
  }

  virtual ~Query() throw() {}

  std::vector<std::string>  rowIDs;
  std::vector<RangeRequest>  ranges;
  int32_t limit;

  _Query__isset __isset;

  void __set_rowIDs(const std::vector<std::string> & val) {
    rowIDs = val;
    __isset.rowIDs = true;
  }

  void __set_ranges(const std::vector<RangeRequest> & val) {
    ranges = val;
    __isset.ranges = true;
  }

  void __set_limit(const int32_t val) {
    limit = val;
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
    if (!(limit == rhs.limit))
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

} // namespace

#endif
