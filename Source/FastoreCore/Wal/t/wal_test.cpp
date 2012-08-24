// $Id$
#include "../Wal.h"
#include "../WalFile.h"

#include <cstdlib>

#include <iostream>

using namespace std;

/*
 * Provoke wald to create a log file and report results.
 */
int
main( int argc, char *argv[] )
{
  int input;
  ostringstream  name;
  name << Wald::responses << getpid();
  string responses(name.str());

  struct stat sb;

  if( -1 == stat(responses.c_str(), &sb) ) {
    cout << "creating response FIFO " << responses << endl;
    if( -1 == mkfifo(responses.c_str(), 0640) ) {
      perror(responses.c_str());
      return EXIT_FAILURE;
    }
  }

  wal_desc_t wal_desc = { "FASTORE", ".LOG", "" };
  
  static const char log_number[ sizeof(wal_desc.log_number) ] = "00000000";

  strcpy( wal_desc.log_number, log_number );
  wal_desc.pid = getpid();
  
  cout << "opening " << Wald::requests << endl;
  int output;
  if( (output = open(Wald::requests, O_WRONLY, 0)) == -1 ) {
    perror(NULL);
    return EXIT_FAILURE;
  }

  cout << "writing " << Wald::requests << endl;
  if( sizeof(wal_desc) != write(output, &wal_desc, sizeof(wal_desc)) ) {
    perror(Wald::requests);
    return EXIT_FAILURE;
  }

  cout << "opening " << responses << endl;
  if( (input = open(responses.c_str(), O_RDONLY, 0)) == -1 ) {
    perror(responses.c_str());
    return EXIT_FAILURE;
  }
 
  cout << "reading " << responses << endl;
  int error;
  if( sizeof(error) != read(input, &error, sizeof(error)) ) {
    perror(NULL);
    return EXIT_FAILURE;
  }

  cout << "read status " << error << " from " << responses << endl;

  return EXIT_SUCCESS;
}
