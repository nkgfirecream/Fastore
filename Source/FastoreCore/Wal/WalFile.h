// $Id$
#pragma once

#include <TransactionID.h>
#include <Comm_types.h>

#include <string>
#include <sstream>

#include <assert.h>
#include <errno.h>
#include <md4.h>
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
  friend unsigned char* operator<<( unsigned char* buf, const Md4& md4 );

 public:
  enum { size = 16 };
  
 private:
  unsigned char data[size]; // md4

 public:
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

struct wal_desc_t
{
  const char prefix[8];
  const char suffix[5];
  char log_number[8];
  pid_t pid;

  std::string name() const {
    std::ostringstream os;
      
    os << prefix << '.' << log_number << suffix;
    return os.str();
  }
};

class WalFile
{
public:
  class Header 
  {
    friend unsigned char* operator<<( unsigned char* p, 
				      const WalFile::Header& h );
    const std::string magic_version;
    int32_t salt;

  public:
    static const int32_t  osPageSize;
    static int32_t sizeInPages() { return WAL_FILE_SIZE / osPageSize; }

    Header();
    size_t Write(int fd);
  };
private:
  enum { WAL_FILE_SIZE = 1L << 30 };

  const std::string dirname;
  const wal_desc_t wal_desc;

  int os_error;

  static int random_fd;
  static const unsigned char *blankPage;

public:
  WalFile( const std::string& dirname, const wal_desc_t& desc );
  
private:
  int osError() const { return os_error; }
  static void randomness(int N, void *P);
};
