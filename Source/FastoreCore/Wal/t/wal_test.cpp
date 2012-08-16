#include "../Wal.h"

#include <cstdlib>

#include <iostream>

using namespace std;


int
main( int argc, char *argv[] )
{
  const string dirname("/tmp");
  const string filename("wal_test");
  NetworkAddress addr;

  Wal wal( dirname, filename, addr );

  

  return EXIT_SUCCESS;
}
