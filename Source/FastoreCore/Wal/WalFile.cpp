#include "WalFile.h"
// $Id$

#include <libgen.h>
#include <openssl/md4.h>
#include <signal.h>

#include <sys/types.h>

#include <cstring>
#include <cstdlib>

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <iterator>

#include "../safe_cast.h"

using namespace std;

#define THROW(msg,value) { \
  ostringstream oops; \
  oops << __FILE__ << ":" << __LINE__  \
  << ": error " << errno << ": " \
  <<  msg << " '" << value << "': " \
  << strerror(errno); \
  throw std::runtime_error( oops.str() ); \
  }

extern const char *random_dev;

namespace Wald {
  const char *requests = "/var/fastore/wald/request";
  const char *responses = "/var/fastore/wald/";
}

wal_desc_t::
wal_desc_t( pid_t pid, const char *prefix, const char *suffix)
  : pid(pid)
{
  if( pid == -1 || pid == 0 )
    THROW("invalid pid", pid);
  if( -1 == kill(pid, 0) ) 
    THROW("invalid pid", pid);

  if( prefix == NULL ) 
    THROW("prefix string required", prefix);
  if( suffix == NULL ) 
    THROW("suffix string required", suffix);
  if( sizeof(this->prefix) - 1 < strlen(prefix)  ) 
    THROW("prefix string exceed max length", sizeof(this->prefix) - 1);
  if( sizeof(this->suffix) - 1 < strlen(suffix)  ) 
    THROW("suffix string exceed max length", sizeof(this->suffix) - 1);

  strcpy(this->prefix, prefix);
  strcpy(this->suffix, suffix);
  memset(this->log_number, '\0', sizeof(this->log_number) );
}

WalFile::Header::Header()
  : magic_version("FASTORE\0")
{
#if 0
  if( osPageSize < 0 ) {
    if( (osPageSize = sysconf(_SC_PAGESIZE)) == (size_t) -1 ) {
      THROW("OS Page size not available", "_SC_PAGESIZE" );
    }
  }
  
  if( WAL_FILE_SIZE % osPageSize != 0 ) {
    THROW("WAL file size not in units of OS page size", osPageSize );
  }
#endif

  randomness( sizeof(salt), &salt );

  if( blankPage == NULL )
	  blankPage = new unsigned char[osPageSize];
}

const int32_t WalFile::Header::osPageSize( SAFE_CAST(int32_t, sysconf(_SC_PAGESIZE)) );

const unsigned char * WalFile::
	blankPage( new unsigned char[WalFile::Header::osPageSize] );

unsigned char*
operator<<( unsigned char* buf, const Md4& md4 )
{
  memcpy( buf, &md4.data, sizeof(md4.data) );
  buf += sizeof(md4.data);

  return buf;
}

unsigned char*
operator<<( unsigned char* buf, const WalFile::Header& h )
{
  unsigned char *p(buf);

  memcpy( p, h.magic_version.c_str(), h.magic_version.size() );
  p += h.magic_version.size();
  
  memcpy( p, &h.osPageSize, sizeof(h.osPageSize) );
  p += sizeof(h.osPageSize);

  memcpy( p, &h.salt, sizeof(h.salt) );
  p += sizeof(h.salt);

  Md4 md4( buf, p - buf );

  return p << md4;
}
  
/*
 * Static
 */
void WalFile::
randomness(int n, void *buf)
{
  if( read(random_fd, buf, n) != n ) {
    THROW("could not use random-number generator", random_dev );
  }
}

int WalFile::random_fd(-1);

/*
 * Public
 */
WalFile::
WalFile( const string& dirname, const wal_desc_t& desc ) 
  : dirname(dirname)
  , wal_desc(desc)
{
  if( random_fd  == -1 ) {
    random_fd = open(random_dev, O_RDONLY);
  }
  if( random_fd  == -1 ) {
    THROW("could not open random-number generator", random_dev );
  }

  // create name directory if need be 
  struct stat sb;
  int fOK;
  if( (fOK = stat(dirname.c_str(), &sb)) == -1 ) {
    if( (fOK = mkdir(dirname.c_str(), 0700)) != 0 ) {
      THROW( "could not create directory name", dirname );
    }
  }

  // construct the WAL filename
  string filename( dirname + "/" + wal_desc.name() );

  // open the WAL file
  static const int flags = (O_WRONLY | O_CREAT | O_EXCL);
  int fd;
  if( (fd = open( filename.c_str(), flags, 0600)) == -1 ) {
    THROW( "could not open WAL file", filename );
  }

  unsigned char page[ Header::osPageSize ];
  Header header;

  memset( page, 0, sizeof(page) );
  page << header;

  if( ssize_t(sizeof(page)) != write(fd, page, sizeof(page)) ) {
    THROW( "write to WAL file", filename );
  }

  for( int32_t i=1; i < header.sizeInPages(); i++ ) {
    if( header.osPageSize != write(fd, blankPage, header.osPageSize) ) {
      THROW( "write to WAL file", filename );
    }
  }
}  


