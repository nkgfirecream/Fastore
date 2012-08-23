#pragma once
#include "WalFile.h"
// $Id$

#include <libgen.h>
#include <md4.h>

#include <sys/types.h>

#include <cstring>
#include <cstdlib>

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <iterator>

using namespace std;

#define THROW(msg,value) { \
  ostringstream oops; \
  oops << __FILE__ << ":" << __LINE__  \
  << ": error " << errno << ": " \
  <<  msg << " '" << value << "': " \
  << strerror(errno); \
  throw std::runtime_error( oops.str() ); \
  }

WalFile::Header::Header()
  : magic_version("FASTORE\0")
{
  if( osPageSize < 0 ) {
    if( (osPageSize = sysconf(_SC_PAGESIZE)) == (size_t) -1 ) {
      THROW("OS Page size not available", "_SC_PAGESIZE" );
    }
  }
  
  if( WAL_FILE_SIZE % osPageSize != 0 ) {
    THROW("WAL file size not in units of OS page size", osPageSize );
  }

  randomness( sizeof(salt), &salt );

  if( blankPage == NULL )
    blankPage = new char(WAL_FILE_SIZE);
}

size_t WalFile::Header::osPageSize( sysconf(_SC_PAGESIZE) );

char * WalFile::blankPage( new char(WAL_FILE_SIZE) );

char*
operator<<( char* buf, const Md4& md4 )
{
  memcpy( p, &md4.data, sizeof(md4.data) );
  p += sizeof(md4.data);

  return p;
}

char*
operator<<( char* buf, const WalFile::Header& h )
{
  char *p(buf);

  memcpy( p, h.magic_version.c_str(), h.magic_version.size() );
  p += h.magic_version.size();
  
  memcpy( p, &os_page_size, sizeof(os_page_size) );
  p += sizeof(os_page_size);

  memcpy( p, &salt, sizeof(salt) );
  p += sizeof(salt);

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

int WalFile::random_fd( open(random_dev, O_RDONLY) );

/*
 * Public
 */
WalFile::
WalFile( const string& dirname, const wal_desc_t& desc ) 
  : dirname(dirname)
  , wal_desc(desc)
{
  if( random_fd  == -1 ) {
    THROW("could not open random-number generator", random_dev );
  }

  // directory name must exist
  struct stat sb;
  int fOK;

  // create name directory if need be 
  if( (fOK = stat(dirname.c_str(), &sb)) == -1 ) {
    if( (fOK = mkdir(dirname.c_str(), 0700)) != 0 ) {
      THROW( "could not create directory name", dir );
    }
  }

  // construct the WAL filename
  string filename( dirname + "/" + wal_desc.name() );

  // open the WAL file
  static const int flags = (O_WRONLY | O_CREAT);
  int fd;
  if( (fd = open( filename.c_str(), flags, 0600)) == -1 ) {
    THROW( "could not open WAL file", filename );
  }

  char page[ Header::osPageSize ];
  Header header;

  memset( page, 0, sizeof(page) );
  page << header;

  if( WAL_FILE_SIZE != write(fd, page, sizeof(page)) ) {
    THROW( "write to WAL file", filename );
  }

  for( size_t i=1; i < header.sizeInPages(); i++ ) {
    if( WAL_FILE_SIZE != write(fd, header.blankPage, sizeof(WAL_FILE_SIZE)) ) {
      THROW( "write to WAL file", filename );
    }
  }
}  

WalFile::Status 
WalFile::Write( const TransactionID& transactionID, const Writes& writes )
{

  return OK;
}

WalFile::Status 
WalFile::Flush( const TransactionID& transactionID )
{
  return OK;
}

Writes
WalFile::GetChanges( const ColumnRevisionSet& col_revisions )
{ 
  Writes changes;
  return changes;
}

const ColumnRevisionSet& 
WalFile::Recover( const TransactionID& transactionID, 
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

    fingerprint_t( const unsigned char * input, size_t length ) {
      MD4_CTX context;
      MD4Init(&context);
      MD4Update(&context, input, length);
      MD4Final(this->data, &context);
    }

    bool operator==( const fingerprint_t& that ) {
      return 0 == memcmp(data, that.data, size);
    }
    bool operator==( const unsigned char * that ) {
      return 0 == memcmp(data, that, size);
    }
  };

  fingerprint_t fingerprint;

  header_t()
    : magic_version("FASTORE"), page_size(0), sequence(0), salt1(0), salt2(0)
  {
    WalFile::randomness( sizeof(salt1), &salt1 );
    WalFile::randomness( sizeof(salt2), &salt2 );

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

  unsigned char * copyToWalFile( unsigned char *wal ) 
  {
    DEBUG_FUNC();
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

  unsigned char * copyFromWalFile( unsigned char *wal ) 
  {
    DEBUG_FUNC();
    string wal_magic( reinterpret_cast<char*>(wal), magic_version.size());

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

/*
 * A "page" (actually just a record) header 
 * has a transaction ID and a length.  The length is inclusive of
 * both the header and the MD4 trailer.  The succeeding record 
 * begins at length + 1.  The end of WAL is marked by a page whose 
 * transaction ID is 0. 
 */

struct page_header_t
{
  int64_t transaction_id;
  size_t  length;

  page_header_t()
    : transaction_id(0), length(0)
  {}

  unsigned char * copyToWalFile( unsigned char *wal ) const
  {
    DEBUG_FUNC();
    memcpy(wal, &transaction_id, sizeof(transaction_id));
    wal += sizeof(transaction_id);
    memcpy(wal, &length, sizeof(length));
    wal += sizeof(length);
    return wal;
  }

  const unsigned char * copyFromWalFile( const unsigned char *wal ) {
    DEBUG_FUNC();
    memcpy(&transaction_id, wal, sizeof(transaction_id));
    wal += sizeof(transaction_id);
    memcpy(&length, wal, sizeof(length));
    wal += sizeof(length);
    return wal;
  }

};

void
WalFile::init( unsigned char *wal )
{
  DEBUG_FUNC();
  memset( wal, 0, WAL_FILE_SIZE );
  this->current = header.copyToWalFile(wal);
  if( -1 == msync(wal, current - wal, MS_SYNC) ) {
    THROW( "msync(2) failed", __FUNCTION__ );
  } 
}
  

bool
WalFile::verify()
{
  DEBUG_FUNC();
  header_t header;

  header.copyFromWalFile(wal);
  if( !header.verify() )
    return false;

  this->current = find_tail();
  return this->current != 0;
}

unsigned char *
WalFile::
verify_page( const unsigned char *data, size_t length ) 
{
  DEBUG_FUNC();
  assert(data != NULL);

  page_header_t header;
  const unsigned char *p = data;
  assert(p != NULL);

  p = header.copyFromWalFile(p);
  p += header.length;
  
  header_t::fingerprint_t computed(data, length);
  unsigned const char * recorded = p - header_t::fingerprint_t::size;

  if( computed == recorded ) {
    return const_cast<unsigned char *>(p);
  }
  return NULL;
}

unsigned char *
WalFile::
find_tail() const
{
  DEBUG_FUNC();
  page_header_t header;
  unsigned char *p = this->current; 

  while(p < wal + WAL_FILE_SIZE) {
    header.copyFromWalFile(p);
    if( header.transaction_id == 0 )
      return p;
    if( (p = verify_page(p, header.length)) == NULL ) {
      return NULL;
    }
  }
  assert(p < wal + WAL_FILE_SIZE);
    
  return NULL;
}
