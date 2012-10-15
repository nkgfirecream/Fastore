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

typedef struct _ServiceStartup__isset {
  _ServiceStartup__isset() : address(false), port(false), workerPaths(false) {}
  bool address;
  bool port;
  bool workerPaths;
} _ServiceStartup__isset;

class ServiceStartup {
 public:

  static const char* ascii_fingerprint; // = "51B15F6CDA6A57A532EE47E861B4AB30";
  static const uint8_t binary_fingerprint[16]; // = {0x51,0xB1,0x5F,0x6C,0xDA,0x6A,0x57,0xA5,0x32,0xEE,0x47,0xE8,0x61,0xB4,0xAB,0x30};

  ServiceStartup() : path(), address(), port(0) {
  }

  virtual ~ServiceStartup() throw() {}

  Path path;
  std::string address;
  int64_t port;
  std::vector<std::string>  workerPaths;

  _ServiceStartup__isset __isset;

  void __set_path(const Path& val) {
    path = val;
  }

  void __set_address(const std::string& val) {
    address = val;
    __isset.address = true;
  }

  void __set_port(const int64_t val) {
    port = val;
    __isset.port = true;
  }

  void __set_workerPaths(const std::vector<std::string> & val) {
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

typedef struct _ServiceConfig__isset {
  _ServiceConfig__isset() : address(false), joinedHive(false) {}
  bool address;
  bool joinedHive;
} _ServiceConfig__isset;

class ServiceConfig {
 public:

  static const char* ascii_fingerprint; // = "E1071F2FF4A36825C7AAB4E06C973AED";
  static const uint8_t binary_fingerprint[16]; // = {0xE1,0x07,0x1F,0x2F,0xF4,0xA3,0x68,0x25,0xC7,0xAA,0xB4,0xE0,0x6C,0x97,0x3A,0xED};

  ServiceConfig() : path() {
  }

  virtual ~ServiceConfig() throw() {}

  Path path;
  std::vector<Path>  workerPaths;
   ::fastore::communication::NetworkAddress address;
   ::fastore::communication::HiveState joinedHive;

  _ServiceConfig__isset __isset;

  void __set_path(const Path& val) {
    path = val;
  }

  void __set_workerPaths(const std::vector<Path> & val) {
    workerPaths = val;
  }

  void __set_address(const  ::fastore::communication::NetworkAddress& val) {
    address = val;
    __isset.address = true;
  }

  void __set_joinedHive(const  ::fastore::communication::HiveState& val) {
    joinedHive = val;
    __isset.joinedHive = true;
  }

  bool operator == (const ServiceConfig & rhs) const
  {
    if (!(path == rhs.path))
      return false;
    if (!(workerPaths == rhs.workerPaths))
      return false;
    if (__isset.address != rhs.__isset.address)
      return false;
    else if (__isset.address && !(address == rhs.address))
      return false;
    if (__isset.joinedHive != rhs.__isset.joinedHive)
      return false;
    else if (__isset.joinedHive && !(joinedHive == rhs.joinedHive))
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
