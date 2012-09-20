#include "Wal.h"
// $Id$

#include <sys/types.h>
#include <openssl/md4.h>

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

const char *random_dev = "/dev/random";

void 
Wal::
randomness(int n, void *buf)
{
  if( read(random_fd, buf, n) != n ) {
    THROW("could not use random-number generator", random_dev );
   }
}

int Wal::random_fd( open(random_dev, O_RDONLY) );

Wal::
Wal( const std::string& dirName, 
	 const string& name, 
     const NetworkAddress& addr )
	: dirName(dirName)
	, name(name)
	, addr(addr)
	, wal(NULL), current(NULL), os_error(0)
{
   if( random_fd  == -1 ) {
	   THROW("could not open random-number generator", random_dev );
   }

  // directory name must exist
  struct stat sb;
  int fOK;

  if( (fOK = stat(dirName.c_str(), &sb)) == -1 ) {
    THROW("could not find directory name", dirName );
  }

  // create name directory if need be 
  const string dir(dirName + "/" + name);
  if( (fOK = stat(dir.c_str(), &sb)) == -1 ) {
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
  bool finitialized = stat(filename.c_str(), &sb) == 0;
  // open the WAL file
  static const int flags = (O_RDWR | O_CREAT | O_DIRECT);
  int fd;
  if( (fd = open( filename.c_str(), flags, 0600)) == -1 ) {
    THROW( "could not open WAL file", filename );
  }


  // set the WAL file's size
  off_t offset;
  if( (offset = lseek( fd, WAL_FILE_SIZE - 1, SEEK_SET)) == -1 ) {
    THROW( "could not set size for WAL file", filename );
  }
  
  if( 1 != write(fd, "\0", 1) ) {
    THROW( "write to end of WAL file", filename );
  }
  
  // map in the memory
  static const int wal_prot = (PROT_READ | PROT_WRITE);
  this->wal = static_cast<unsigned char*>( mmap(NULL, WAL_FILE_SIZE, wal_prot, 
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

    fingerprint_t( const unsigned char * input, size_t length ) {
      MD4_CTX context;
      MD4_Init(&context);
      MD4_Update(&context, input, length);
      MD4_Final(this->data, &context);
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
    Wal::randomness( sizeof(salt1), &salt1 );
    Wal::randomness( sizeof(salt2), &salt2 );

    mem[0] = &page_size;
    mem[1] = &sequence;
    mem[2] = &salt1;
    mem[3] = &salt2;
  }

  fingerprint_t& compute_stamp( fingerprint_t& output ) const
  { 
    MD4_CTX context;

    MD4_Init(&context);

    MD4_Update(&context, 
	      reinterpret_cast<const unsigned char*>(magic_version.c_str()), 
	      magic_version.size() );

    const unsigned char * pend 
      = reinterpret_cast<const unsigned char*>(mem) 
      + sizeof(mem)/sizeof(mem[0]);
    for( const unsigned char *p = reinterpret_cast<const unsigned char*>(mem);  
	 p <  pend; p += sizeof(mem[0]) ) {
      MD4_Update(&context, p, sizeof(mem[0]) );
    }
    
    MD4_Final(output.data, &context);
    
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

  unsigned char * copyToWal( unsigned char *wal ) 
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

  unsigned char * copyFromWal( unsigned char *wal ) 
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

  unsigned char * copyToWal( unsigned char *wal ) const
  {
    DEBUG_FUNC();
    memcpy(wal, &transaction_id, sizeof(transaction_id));
    wal += sizeof(transaction_id);
    memcpy(wal, &length, sizeof(length));
    wal += sizeof(length);
    return wal;
  }

  const unsigned char * copyFromWal( const unsigned char *wal ) {
    DEBUG_FUNC();
    memcpy(&transaction_id, wal, sizeof(transaction_id));
    wal += sizeof(transaction_id);
    memcpy(&length, wal, sizeof(length));
    wal += sizeof(length);
    return wal;
  }

};

void
Wal::init( unsigned char *wal )
{
  DEBUG_FUNC();
  memset( wal, 0, WAL_FILE_SIZE );
  this->current = header.copyToWal(wal);
  if( -1 == msync(wal, current - wal, MS_SYNC) ) {
    THROW( "msync(2) failed", __FUNCTION__ );
  } 
}
  

bool
Wal::verify()
{
  DEBUG_FUNC();
  header_t header;

  header.copyFromWal(wal);
  if( !header.verify() )
    return false;

  this->current = find_tail();
  return this->current != 0;
}

unsigned char *
Wal::
verify_page( const unsigned char *data, size_t length ) 
{
  DEBUG_FUNC();
  assert(data != NULL);

  page_header_t header;
  const unsigned char *p = data;
  assert(p != NULL);

  p = header.copyFromWal(p);
  p += header.length;
  
  header_t::fingerprint_t computed(data, length);
  unsigned const char * recorded = p - header_t::fingerprint_t::size;

  if( computed == recorded ) {
    return const_cast<unsigned char *>(p);
  }
  return NULL;
}

unsigned char *
Wal::
find_tail() const
{
  DEBUG_FUNC();
  page_header_t header;
  unsigned char *p = this->current; 

  while(p < wal + WAL_FILE_SIZE) {
    header.copyFromWal(p);
    if( header.transaction_id == 0 )
      return p;
    if( (p = verify_page(p, header.length)) == NULL ) {
      return NULL;
    }
  }
  assert(p < wal + WAL_FILE_SIZE);
    
  return NULL;
}
