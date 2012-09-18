// $Id$
#pragma once

#include <TransactionID.h>
#include <Comm_types.h>

#include <string>
#include <sstream>

#include <assert.h>
#include <errno.h>
#include <openssl/md4.h>
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

namespace Wald {
  extern const char *requests;
  extern const char *responses;
}

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
    MD4_Init(&context);
    MD4_Update(&context, input, length);
    MD4_Final(this->data, &context);
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
  // lengths allow for NULLs
  char prefix[7+1];
  char suffix[3+1];
  char log_number[8+1]; 
  pid_t pid;

  wal_desc_t( pid_t pid = getpid(), 
	      const char *prefix = "FASTORE", 
	      const char *suffix = "LOG" );

  std::string name() const {
    std::ostringstream os;
      
    os << prefix << '.' 
       << std::string(log_number, sizeof(log_number)-1) 
       << '.' << suffix;
    return os.str();
  }

  void lsn( size_t value ) {
      sprintf(log_number, "%08zu", value);
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
