/**
 * Autogenerated by Thrift Compiler (0.9.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef Server_TYPES_H
#define Server_TYPES_H

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include "Comm_types.h"


namespace fastore { namespace server {

typedef std::string Path;

typedef int32_t WorkerNumber;

typedef struct _ServiceStartup__isset {
  _ServiceStartup__isset() : address(false), port(false), workerPaths(false) {}
  bool address;
  bool port;
  bool workerPaths;
} _ServiceStartup__isset;

class ServiceStartup {
 public:

  static const char* ascii_fingerprint; // = "B04C5F620A67663D1E5AAA3FB7A73C67";
  static const uint8_t binary_fingerprint[16]; // = {0xB0,0x4C,0x5F,0x62,0x0A,0x67,0x66,0x3D,0x1E,0x5A,0xAA,0x3F,0xB7,0xA7,0x3C,0x67};

  ServiceStartup() : path(), address(), port(0) {
  }

  virtual ~ServiceStartup() throw() {}

  Path path;
  std::string address;
  int32_t port;
  std::map<WorkerNumber, std::string>  workerPaths;

  _ServiceStartup__isset __isset;

  void __set_path(const Path& val) {
    path = val;
  }

  void __set_address(const std::string& val) {
    address = val;
    __isset.address = true;
  }

  void __set_port(const int32_t val) {
    port = val;
    __isset.port = true;
  }

  void __set_workerPaths(const std::map<WorkerNumber, std::string> & val) {
    workerPaths = val;
    __isset.workerPaths = true;
  }

  bool operator == (const ServiceStartup & rhs) const
  {
    if (!(path == rhs.path))
      return false;
    if (__isset.address != rhs.__isset.address)
      return false;
    else if (__isset.address && !(address == rhs.address))
      return false;
    if (__isset.port != rhs.__isset.port)
      return false;
    else if (__isset.port && !(port == rhs.port))
      return false;
    if (__isset.workerPaths != rhs.__isset.workerPaths)
      return false;
    else if (__isset.workerPaths && !(workerPaths == rhs.workerPaths))
      return false;
    return true;
  }
  bool operator != (const ServiceStartup &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ServiceStartup & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(ServiceStartup &a, ServiceStartup &b);


class JoinedTopology {
 public:

  static const char* ascii_fingerprint; // = "C7A49BCE8D210A50A152CFEB94E29623";
  static const uint8_t binary_fingerprint[16]; // = {0xC7,0xA4,0x9B,0xCE,0x8D,0x21,0x0A,0x50,0xA1,0x52,0xCF,0xEB,0x94,0xE2,0x96,0x23};

  JoinedTopology() : topologyID(0), hostID(0) {
  }

  virtual ~JoinedTopology() throw() {}

   ::fastore::communication::TopologyID topologyID;
   ::fastore::communication::HostID hostID;
  std::map<WorkerNumber,  ::fastore::communication::PodID>  workerPodIDs;

  void __set_topologyID(const  ::fastore::communication::TopologyID val) {
    topologyID = val;
  }

  void __set_hostID(const  ::fastore::communication::HostID val) {
    hostID = val;
  }

  void __set_workerPodIDs(const std::map<WorkerNumber,  ::fastore::communication::PodID> & val) {
    workerPodIDs = val;
  }

  bool operator == (const JoinedTopology & rhs) const
  {
    if (!(topologyID == rhs.topologyID))
      return false;
    if (!(hostID == rhs.hostID))
      return false;
    if (!(workerPodIDs == rhs.workerPodIDs))
      return false;
    return true;
  }
  bool operator != (const JoinedTopology &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const JoinedTopology & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(JoinedTopology &a, JoinedTopology &b);

typedef struct _ServiceConfig__isset {
  _ServiceConfig__isset() : joinedTopology(false) {}
  bool joinedTopology;
} _ServiceConfig__isset;

class ServiceConfig {
 public:

  static const char* ascii_fingerprint; // = "D3217A03F7C15A8AA899518E3E42EE89";
  static const uint8_t binary_fingerprint[16]; // = {0xD3,0x21,0x7A,0x03,0xF7,0xC1,0x5A,0x8A,0xA8,0x99,0x51,0x8E,0x3E,0x42,0xEE,0x89};

  ServiceConfig() : path() {
  }

  virtual ~ServiceConfig() throw() {}

  Path path;
  std::map<WorkerNumber, Path>  workerPaths;
   ::fastore::communication::NetworkAddress address;
  JoinedTopology joinedTopology;

  _ServiceConfig__isset __isset;

  void __set_path(const Path& val) {
    path = val;
  }

  void __set_workerPaths(const std::map<WorkerNumber, Path> & val) {
    workerPaths = val;
  }

  void __set_address(const  ::fastore::communication::NetworkAddress& val) {
    address = val;
  }

  void __set_joinedTopology(const JoinedTopology& val) {
    joinedTopology = val;
    __isset.joinedTopology = true;
  }

  bool operator == (const ServiceConfig & rhs) const
  {
    if (!(path == rhs.path))
      return false;
    if (!(workerPaths == rhs.workerPaths))
      return false;
    if (!(address == rhs.address))
      return false;
    if (__isset.joinedTopology != rhs.__isset.joinedTopology)
      return false;
    else if (__isset.joinedTopology && !(joinedTopology == rhs.joinedTopology))
      return false;
    return true;
  }
  bool operator != (const ServiceConfig &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ServiceConfig & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(ServiceConfig &a, ServiceConfig &b);

}} // namespace

#endif
