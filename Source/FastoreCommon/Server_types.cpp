/**
 * Autogenerated by Thrift Compiler (0.9.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "Server_types.h"

#include <algorithm>

namespace fastore { namespace server {

const char* ServiceStartup::ascii_fingerprint = "51B15F6CDA6A57A532EE47E861B4AB30";
const uint8_t ServiceStartup::binary_fingerprint[16] = {0x51,0xB1,0x5F,0x6C,0xDA,0x6A,0x57,0xA5,0x32,0xEE,0x47,0xE8,0x61,0xB4,0xAB,0x30};

uint32_t ServiceStartup::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_path = false;

  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->path);
          isset_path = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->address);
          this->__isset.address = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_I64) {
          xfer += iprot->readI64(this->port);
          this->__isset.port = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->workerPaths.clear();
            uint32_t _size0;
            ::apache::thrift::protocol::TType _etype3;
            iprot->readListBegin(_etype3, _size0);
            this->workerPaths.resize(_size0);
            uint32_t _i4;
            for (_i4 = 0; _i4 < _size0; ++_i4)
            {
              xfer += iprot->readString(this->workerPaths[_i4]);
            }
            iprot->readListEnd();
          }
          this->__isset.workerPaths = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  if (!isset_path)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t ServiceStartup::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("ServiceStartup");

  xfer += oprot->writeFieldBegin("path", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->path);
  xfer += oprot->writeFieldEnd();

  if (this->__isset.address) {
    xfer += oprot->writeFieldBegin("address", ::apache::thrift::protocol::T_STRING, 2);
    xfer += oprot->writeString(this->address);
    xfer += oprot->writeFieldEnd();
  }
  if (this->__isset.port) {
    xfer += oprot->writeFieldBegin("port", ::apache::thrift::protocol::T_I64, 3);
    xfer += oprot->writeI64(this->port);
    xfer += oprot->writeFieldEnd();
  }
  if (this->__isset.workerPaths) {
    xfer += oprot->writeFieldBegin("workerPaths", ::apache::thrift::protocol::T_LIST, 4);
    {
      xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRING, static_cast<uint32_t>(this->workerPaths.size()));
      std::vector<std::string> ::const_iterator _iter5;
      for (_iter5 = this->workerPaths.begin(); _iter5 != this->workerPaths.end(); ++_iter5)
      {
        xfer += oprot->writeString((*_iter5));
      }
      xfer += oprot->writeListEnd();
    }
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(ServiceStartup &a, ServiceStartup &b) {
  using ::std::swap;
  swap(a.path, b.path);
  swap(a.address, b.address);
  swap(a.port, b.port);
  swap(a.workerPaths, b.workerPaths);
  swap(a.__isset, b.__isset);
}

const char* ServiceConfig::ascii_fingerprint = "E1071F2FF4A36825C7AAB4E06C973AED";
const uint8_t ServiceConfig::binary_fingerprint[16] = {0xE1,0x07,0x1F,0x2F,0xF4,0xA3,0x68,0x25,0xC7,0xAA,0xB4,0xE0,0x6C,0x97,0x3A,0xED};

uint32_t ServiceConfig::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_path = false;
  bool isset_workerPaths = false;

  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->path);
          isset_path = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->workerPaths.clear();
            uint32_t _size6;
            ::apache::thrift::protocol::TType _etype9;
            iprot->readListBegin(_etype9, _size6);
            this->workerPaths.resize(_size6);
            uint32_t _i10;
            for (_i10 = 0; _i10 < _size6; ++_i10)
            {
              xfer += iprot->readString(this->workerPaths[_i10]);
            }
            iprot->readListEnd();
          }
          isset_workerPaths = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRUCT) {
          xfer += this->address.read(iprot);
          this->__isset.address = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_STRUCT) {
          xfer += this->joinedHive.read(iprot);
          this->__isset.joinedHive = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  if (!isset_path)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_workerPaths)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t ServiceConfig::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("ServiceConfig");

  xfer += oprot->writeFieldBegin("path", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->path);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("workerPaths", ::apache::thrift::protocol::T_LIST, 2);
  {
    xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRING, static_cast<uint32_t>(this->workerPaths.size()));
    std::vector<Path> ::const_iterator _iter11;
    for (_iter11 = this->workerPaths.begin(); _iter11 != this->workerPaths.end(); ++_iter11)
    {
      xfer += oprot->writeString((*_iter11));
    }
    xfer += oprot->writeListEnd();
  }
  xfer += oprot->writeFieldEnd();

  if (this->__isset.address) {
    xfer += oprot->writeFieldBegin("address", ::apache::thrift::protocol::T_STRUCT, 3);
    xfer += this->address.write(oprot);
    xfer += oprot->writeFieldEnd();
  }
  if (this->__isset.joinedHive) {
    xfer += oprot->writeFieldBegin("joinedHive", ::apache::thrift::protocol::T_STRUCT, 4);
    xfer += this->joinedHive.write(oprot);
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(ServiceConfig &a, ServiceConfig &b) {
  using ::std::swap;
  swap(a.path, b.path);
  swap(a.workerPaths, b.workerPaths);
  swap(a.address, b.address);
  swap(a.joinedHive, b.joinedHive);
  swap(a.__isset, b.__isset);
}

}} // namespace