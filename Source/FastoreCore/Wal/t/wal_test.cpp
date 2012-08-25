// $Id$
/*
 * Provoke wald to create a log file and report results.
 * $Id$
 */
#include "../Wal.h"
#include "../WalFile.h"

#include <dirent.h>

#include <cstdio>
#include <cstdlib>

#include <iostream>

using namespace std;

/*
 * Parse a filename for its sequence number. 
 * Return next in sequence.
 */
size_t
lsn( const dirent& ent )
{
  string name( ent.d_name, ent.d_namlen );
  size_t begin(name.size()), end;
  for( int i=0; i < 2 && 0 < begin && begin != string::npos; i++ ) {
    end = begin - 1;
    begin = name.find_last_of('.', end);
  }
  if( begin == string::npos ) {
    cerr << __LINE__ << ": could not parse LSN from '" << name << "'\n";
    return 0;
  }
  assert(begin < end);
  istringstream input( name.substr(++begin, end) );
  size_t output(0);
  input >> output;
  return ++output;
}

int
main( int argc, char *argv[] )
{
  bool fWait(true);
  std::string logdir("/var/fastore");
  int input;
  ostringstream  name;
  name << Wald::responses << getpid();
  string responses(name.str());

  extern char *optarg;
  extern int optind;
  int bflag, ch, fd;

  bflag = 0;
  while ((ch = getopt(argc, argv, "xd:")) != -1) {
    switch (ch) {
    case 'd':
      logdir = optarg;
      break;
    case 'x':
      fWait = false;
      break;
    case '?':
    default:
      cerr << "syntax: wal_test [-x] [-d logdir]\n";
      return EXIT_FAILURE;
    }
  }
  argc -= optind;
  argv += optind;

  struct stat sb;

  if( -1 == stat(responses.c_str(), &sb) ) {
    cout << "creating response FIFO " << responses << endl;
    if( -1 == mkfifo(responses.c_str(), 0640) ) {
      perror(responses.c_str());
      return EXIT_FAILURE;
    }
  }

  /* 
   * Find the last WAL file in the directory and 
   * determine the current LSN.
   */
  DIR *d;
  if( (d = opendir(logdir.c_str())) == NULL ) {
    perror(logdir.c_str());
    return EXIT_FAILURE;
  }
  struct dirent *pent(NULL), *p;
  while( (p = readdir(d)) != NULL) {
    static const char prefix[] = "FASTORE.";
    if( p->d_namlen < sizeof(prefix) )
      continue;
    if( 0 == strncmp(p->d_name, prefix, sizeof(prefix)-1) ) {
      pent = p;
    }
  }

  wal_desc_t wal_desc;
  wal_desc.lsn( pent? lsn(*pent) : 0 );

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
 
  int error;
  if( fWait ) {
    cout << "reading " << responses << endl;
    if( sizeof(error) != read(input, &error, sizeof(error)) ) {
      perror(responses.c_str());
      if( -1 == unlink(responses.c_str()) ) {
	perror(responses.c_str());
      }
      return EXIT_FAILURE;
    }
    cout << "read status " << error << " from " << responses << endl;
  } else {
    sleep(1); // let daemon open the request FIFO before we delete it
  }

  if( -1 == unlink(responses.c_str()) ) {
    perror(responses.c_str());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
