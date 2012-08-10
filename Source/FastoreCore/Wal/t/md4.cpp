/*
 * test md4 performance
 * $Id$
 */
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <climits>
#include <ctime>
#include <cstring>

#include <errno.h>
#include <fcntl.h>
#include <md4.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

using namespace std; 

#define THROW(msg,value) { \
  ostringstream oops; \
  oops << __FILE__ << ":" << __LINE__  \
  << ": error " << errno << ": " \
  <<  msg << " '" << value << "': " \
  << strerror(errno); \
  throw std::runtime_error( oops.str() ); \
  }

int main( int argc, char *argv[] )
{
  if( argc < 2 ) return EXIT_FAILURE;

  const char *filename(argv[1]);

  struct stat sb;
  int fOK;

  if( (fOK = stat(filename, &sb)) == -1 ) {
    THROW("could not find file", filename );
  }

  unsigned int len(sb.st_size);
  cout << filename << " is " << len << " bytes\n";

  static const int flags = (O_RDWR | O_CREAT | O_DIRECT);
  int fd;
  if( (fd = open( filename, flags, 0600)) == -1 ) {
    THROW( "could not open file", filename );
  }

  static const int wal_prot = (PROT_READ | PROT_WRITE);
  unsigned char * data = 
    static_cast<unsigned char*>( mmap(NULL, len, wal_prot, MAP_FILE, fd, 0) );
  if( data == MAP_FAILED ) {
    THROW( "could not map WAL file", filename );
  }

  char digest[3 * 12];
  
  time_t start; 
  time(&start);
  int i(0);
  len = 4096;
  while( time(0) < start + 100 && i < LONG_MAX) {
    MD4Data(data, len, digest);
    i++;
  }
  cout << i << " iterations in " << (time(0) - start) << " seconds, "
       << (i / (time(0) - start)) << "iter/sec, " 
       << ((double)(time(0) - start) / i) << " second/iteration" 
       << '\n';

  return EXIT_SUCCESS;
}
