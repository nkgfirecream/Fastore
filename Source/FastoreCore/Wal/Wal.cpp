#pragma once
#include "Wal.h"
#include "sqlite3/sqliteInt.parts.h"
// $Id$

#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include <cassert>
#include <ctime>
#include <cstring>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>


/*
 * The argument to this macro must be of type u32. On a little-endian
 * architecture, it returns the u32 value that results from interpreting
 * the 4 bytes as a big-endian value. On a big-endian architecture, it
 * returns the value that would be produced by intepreting the 4 bytes
 * of the input value as a little-endian integer.
 */
#define BYTESWAP32(x) ( \
    (((x)&0x000000FF)<<24) + (((x)&0x0000FF00)<<8)  \
  + (((x)&0x00FF0000)>>8)  + (((x)&0xFF000000)>>24) \
)

/*
 * Generate or extend an 8 byte checksum based on the data in 
 * array aByte[] and the initial values of aIn[0] and aIn[1] (or
 * initial values of 0 and 0 if aIn==NULL).
 *
 * The checksum is written back into aOut[] before returning.
 *
 * nByte must be a positive multiple of 8.
 */
static void walChecksumBytes(
  int nativeCksum, /* True for native byte-order, false for non-native */
  u8 *a,           /* Content to be checksummed */
  int nByte,       /* Bytes of content in a[].  Must be a multiple of 8. */
  const u32 *aIn,  /* Initial checksum value input */
  u32 *aOut        /* OUT: Final checksum value output */
)
{
  u32 s1, s2;
  u32 *aData = (u32 *)a;
  u32 *aEnd = (u32 *)&a[nByte];

  if( aIn ) {
    s1 = aIn[0];
    s2 = aIn[1];
  } else {
    s1 = s2 = 0;
  }

  assert( nByte>=8 );
  assert( (nByte&0x00000007)==0 );

  if( nativeCksum ) {
    do {
      s1 += *aData++ + s2;
      s2 += *aData++ + s1;
    } while( aData<aEnd );
  } else {

    do {
      s1 += BYTESWAP32(aData[0]) + s2;
      s2 += BYTESWAP32(aData[1]) + s1;
      aData += 2;
    } while( aData<aEnd );
  }

  aOut[0] = s1;
  aOut[1] = s2;
}

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
 * The constructor also takes the worker's address.  Wal 
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

Wal::
Wal( const std::string& dirName, const string& name, 
     const NetworkAddress& addr )
  : dirName(dirName), name(name), addr(addr)
  , wal(NULL)
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
  
}  

Wal::Status 
Wal::Write( const TransactionID& transactionID, const ChangeSet& changes )
{

  return OK;
}

Wal::Status 
Wal::Flush( const TransactionID& transactionID )
{
  return OK;
}

ChangeSet 
Wal::GetChanges( const ColumnRevisionSet& col_revisions )
{ 
  ChangeSet changes;
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
 *    24: Checksum-1 (first part of checksum for first 24 bytes of header).
 *    28: Checksum-2 (second part of checksum for first 24 bytes of header).
*/
void
Wal::VerifyFile()
{
}
