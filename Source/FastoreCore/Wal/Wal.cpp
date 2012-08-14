#pragma once
#include "Wal.h"
#include "sqlite3/sqliteInt.parts.h"
// $Id$

#include <sys/types.h>
#include <md4.h>

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <iterator>

using namespace std;

/**
 * The Wal constructor takes the name of a directory where WAL files
 * are kept, and a name -- its own name -- that it will use as a name
 * of a subirectory where it will keep its WAL files Each file is up
 * to 1 GB (TODO: tunable).  The naming convention is:
 * 	name/fastore.yyyy-mm-dd.wal.seq 
 * where seq is a monotonically increasing number since startup, regardless
 * of date. The current (active) WAL has no seq suffix.
 *
 * A good name for a Wal object might be just an integer representing 
 * the processor core that justifies the Wal's existence.  
 */

/**
 * The constructor also takes the worker's tcp/ip address.  Wal 
 * forms a connection to it, where it sends flush confirmations.
 */
#define THROW(msg,value) { \
  ostringstream oops; \
  oops << __FILE__ << ":" << __LINE__  \
  << ": error " << errno << ": " \
  <<  msg << " '" << value << "': " \
  << strerror(errno); \
  throw std::runtime_error( oops.str() ); \
  }

/*
** CAPI3REF: Pseudo-Random Number Generator
**
** SQLite contains a high-quality pseudo-random number generator (PRNG) used to
** select random [ROWID | ROWIDs] when inserting new records into a table that
** already uses the largest possible [ROWID].  The PRNG is also used for
** the build-in random() and randomblob() SQL functions.  This interface allows
** applications to access the same PRNG for other purposes.
**
** ^A call to this routine stores N bytes of randomness into buffer P.
**
** ^The first time this routine is invoked (either internally or by
** the application) the PRNG is seeded using randomness obtained
** from the xRandomness method of the default [sqlite3_vfs] object.
** ^On all subsequent invocations, the pseudo-randomness is generated
** internally and without recourse to the [sqlite3_vfs] xRandomness
** method.
*/
#define SQLITE_API 
SQLITE_API void sqlite3_randomness(int N, void *P);

Wal::
Wal( const std::string& dirName, const string& name, 
     const NetworkAddress& addr )
  : dirName(dirName), name(name), addr(addr)
  , wal(NULL), current(NULL), os_error(0)
 {
  // directory name must exist
  struct stat sb;
  int fOK;

  if( (fOK = stat(dirName.c_str(), &sb)) == -1 ) {
    THROW("could not find directory name", dirName );
  }

  // create name directory if need be 
  const string dir(dirName + "/" + name);
  if( (fOK = stat(dirName.c_str(), &sb)) == -1 ) {
    if( (fOK = mkdir(dir.c_str(), 0700)) != 0 ) {
      THROW( "could not create directory name", dir );
    }
  }

  // construct the WAL filename
  time_t now = time(NULL);
  if( -1 == (ssize_t)now) {
    THROW( "could not establish time in", wal );
  } 

  struct tm *pnow = localtime(&now);
  ostringstream date;
  date << right << 1900 + pnow->tm_year << '-' 
       << setw(2) << setfill('0') << pnow->tm_mon << '-' 
       << setw(2) << setfill('0') << pnow->tm_mday;

  const string filename( dir + "/fastore." + date.str() + ".wal" );

  // does the WAL exist? 
  bool finitialized = stat(filename.c_str(), &sb) != -1;
  // open the WAL file
  static const int flags = (O_RDWR | O_CREAT | O_DIRECT);
  int fd;
  if( (fd = open( filename.c_str(), flags, 0600)) == -1 ) {
    THROW( "could not open WAL file", filename );
  }

  // set the WAL file's size
  off_t offset;
  if( (offset = lseek( fd, WAL_FILE_SIZE, SEEK_SET)) == -1 ) {
    THROW( "could not set size for WAL file", filename );
  }

  // map in the memory
  static const int wal_prot = (PROT_READ | PROT_WRITE);
  this->wal = static_cast<char*>( mmap(NULL, WAL_FILE_SIZE, wal_prot, 
				       MAP_FILE, fd, 0) );
  if( wal == MAP_FAILED ) {
    THROW( "could not map WAL file", filename );
  }
  
  // verify WAL file and set statistics
  if( !finitialized )
    init(wal);

  if( !verify() ) {
    THROW( "WAL file invalid", filename );
  } 
}  

Wal::Status 
Wal::Write( const TransactionID& transactionID, const Writes& writes )
{

  return OK;
}

Wal::Status 
Wal::Flush( const TransactionID& transactionID )
{
  return OK;
}

Writes
Wal::GetChanges( const ColumnRevisionSet& col_revisions )
{ 
  Writes changes;
  return changes;
}

const ColumnRevisionSet& 
Wal::Recover( const TransactionID& transactionID, 
	      ColumnRevisionSet& revisionSet /* OUT */ )
{
  return revisionSet;
}

/// Private functions ///

/*
 * The WAL header is 32 bytes in size and consists of the following eight
 * big-endian 32-bit unsigned integer values:
 *
 *     0: Magic number.  0x464153544 ("FAST")
 *     4: File format version.  Currently 0xf524500 ("ORE\0") 
 *     8: OS page size.  Example: 4096
 *    12: Checkpoint sequence number
 *    16: Salt-1, random integer incremented with each checkpoint
 *    20: Salt-2, a different random integer changing with each ckpt
 *    24-40 MD4 fingerprint
 */

struct header_t
{
  const string magic_version;
  int32_t page_size, sequence, salt1, salt2;
  int32_t *mem[4];
  struct fingerprint_t {
    enum { size = 16 };
    unsigned char data[size]; // md4

    fingerprint_t() {
      memset(data, 0, size);
    }

    bool operator==( const fingerprint_t& that ) {
      return 0 == memcmp(data, that.data, size);
    }
  };
  fingerprint_t fingerprint;

  header_t()
    : magic_version("FASTORE"), page_size(0), sequence(0), salt1(0), salt2(0)
  {
    sqlite3_randomness( sizeof(salt1), &salt1 );
    sqlite3_randomness( sizeof(salt2), &salt2 );

    mem[0] = &page_size;
    mem[1] = &sequence;
    mem[2] = &salt1;
    mem[3] = &salt2;
  }

  fingerprint_t& compute_stamp( fingerprint_t& output ) const
  { 
    MD4_CTX context;

    MD4Init(&context);

    MD4Update(&context, 
	      reinterpret_cast<const unsigned char*>(magic_version.c_str()), 
	      magic_version.size() );

    const unsigned char * pend 
      = reinterpret_cast<const unsigned char*>(mem) 
      + sizeof(mem)/sizeof(mem[0]);
    for( const unsigned char *p = reinterpret_cast<const unsigned char*>(mem);  
	 p <  pend; p += sizeof(mem[0]) ) {
      MD4Update(&context, p, sizeof(mem[0]) );
    }
    
    MD4Final(output.data, &context);
    
    return output;
  }

  void stamp() 
  {
    compute_stamp(this->fingerprint);
  }    

  bool verify()
  {
    fingerprint_t fingerprint;
    return this->fingerprint == compute_stamp(fingerprint);
  }

  char * copyToWal( char *wal ) 
  {
    memcpy( wal, magic_version.c_str(), magic_version.size() );
    wal += magic_version.size();
  
    const unsigned char * pend 
      = reinterpret_cast<const unsigned char*>(mem) 
      + sizeof(mem)/sizeof(mem[0]);

    for( const unsigned char *p = reinterpret_cast<const unsigned char*>(mem); 
	 p < pend; p += sizeof(mem[0]) ) {

      memcpy( wal, p, sizeof(mem[0]) );
      wal += sizeof(mem[0]);
    }
    
    stamp();
    memcpy( wal, fingerprint.data, fingerprint_t::size );

    return wal + fingerprint_t::size;
  }

  char * copyFromWal( char *wal ) 
  {
    string wal_magic( wal, magic_version.size());
    const string& p(magic_version);
    const_cast<string&>(p).swap(wal_magic);
    wal += magic_version.size();
  
    const unsigned char * pend 
      = reinterpret_cast<const unsigned char*>(mem) 
      + sizeof(mem)/sizeof(mem[0]);

    for( unsigned char *p = reinterpret_cast<unsigned char*>(mem); 
	 p < pend; p += sizeof(mem[0]) ) {
      memcpy( p, wal, sizeof(mem[0]) );
      wal += sizeof(mem[0]);
    }
    
    memcpy( fingerprint.data, wal, fingerprint_t::size );

    return wal + fingerprint_t::size;
  }
};

static header_t header;

struct page_header_t
{
  int64_t transaction_id;
  size_t  length;

  page_header_t()
    : transaction_id(0), length(0)
  {}

};

void
Wal::init( char *wal )
{
  memset( wal, 0, WAL_FILE_SIZE );
  this->current = header.copyToWal(wal);
}
  

bool
Wal::verify()
{
  header_t header;
  header.copyFromWal(wal);
  return header.verify();
}


 
