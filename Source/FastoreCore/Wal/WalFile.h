// $Id$
#pragma once

#include <string>
#include <TransactionID.h>
#include <Comm_types.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

///namespace fastore { namespace communication {
using fastore::communication::ColumnID;
using fastore::communication::ColumnWrites;
using fastore::communication::NetworkAddress;
using fastore::communication::Revision;
using fastore::communication::Writes;

#if DEBUG_WAL_FUNC
# define DEBUG_FUNC() { \
    std::cerr << __FUNCTION__  << ":" << __LINE__  << std::endl; }
#else
# define DEBUG_FUNC() {}
#endif

class Md4 
{
 public:
  enum { size = 16 };
  
 private:
  unsigned char data[size]; // md4

  Md4() {
    memset(data, 0, size);
  }

  Md4( const unsigned char * input, size_t length ) {
    MD4_CTX context;
    MD4Init(&context);
    MD4Update(&context, input, length);
    MD4Final(this->data, &context);
  }

  bool operator==( const Md4& that ) {
    return 0 == memcmp(data, that.data, size);
  }
  bool operator==( const unsigned char * that ) {
    return 0 == memcmp(data, that, size);
  }
};


class WalFile
{
public:
  class Header 
  {
    friend char* operator<<( char* p, const WalFile::Header& h );

    const string magic_version;
    int32_t salt;

  public:
    static const in32_t  osPageSize;
    static size_t sizeInPages() { return WAL_FILE_SIZE / osPageSize; }

    Header();
    Write(int fd);
  };
private:
  enum { WAL_FILE_SIZE = 1L << 30 };

  const std::string dirname;
  const wal_desc_t wal_desc;

  int os_error;

  static int random_fd;
  static const char *blankPage;

public:
  WalFile( const std::string& dirname, const wal_desc_t& desc );
  
  int osError() const { return os_error; }
  static void randomness(int N, void *P);

private:
};
