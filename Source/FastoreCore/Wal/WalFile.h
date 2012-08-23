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

class WalFile
{
public:
  class Header 
  {
  public:
    static const size_t osPageSize;
    static const char *blankPage;
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

public:
  WalFile( const std::string& dirname, const wal_desc_t& desc );
  
  int osError() const { return os_error; }
  static void randomness(int N, void *P);

private:
};
