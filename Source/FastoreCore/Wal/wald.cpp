/*
 * $Id$
 * Allocate WAL files.
 * Accept requests over FIFO and respond over another.
 */

#include <cstdlib>
#include <cstring>

#include <iostream>
#include <fstream>
#include <sstream>

#include <map>

#include <fcntl.h>
#include <libgen.h>
#include <syslog.h>
#include <util.h>

#include <sys/stat.h>

#include "WalFile.h"

using namespace std;

wal_desc_t wal_desc = { "FASTORE", ".LOG", "" };

istream&
operator>>( istream& is, wal_desc_t& desc ) 
{
  is >> desc.log_number >> desc.pid;
  return is;
}

static const char requests[] = "/var/fastore/wald/request";
static const char responses[] = "/var/fastore/wald/";

typedef std::map<pid_t, int> pidmap;
pidmap results;

static void usage()
{
  cerr << "syntax: wald [-d dirname]\n"; 
}

static void unlink_pidfile(void)
{
  static const char name[] = "/var/run/wald";
  unlink(name);
}

int
main( int argc, char *argv[] )
{
  std::string dirname("/var/fastore");
  const char *name = basename(argv[0]);

  extern char *optarg;
  extern int optind;
  int bflag, ch, fd;

  bflag = 0;
  while ((ch = getopt(argc, argv, "d:")) != -1) {
    switch (ch) {
    case 'd':
      dirname = optarg;
      break;
    case '?':
    default:
      usage();
    }
  }
  argc -= optind;
  argv += optind;

  struct stat sb;

  // attach to syslogd
  openlog( name, LOG_CONS|LOG_PID, LOG_DAEMON);

  /*
   * Kill parent process, become child of init.
   * Become session leader. 
   * Clear umask. 
   * Change to correct working directory. 
   * Point stdin etc. to /dev/null. 
   */
  pid_t pid = fork();

  if( pid != 0 )
    return EXIT_SUCCESS;

  syslog( LOG_INFO, "%d: %s: started", __LINE__, name);

  if( (pid = setsid()) == -1 ) {
    syslog( LOG_ERR, "%d: %m", __LINE__);
    return EXIT_FAILURE;
  }

  umask(0);

  char req[ sizeof(::requests) ];
  strcpy(req, ::requests);
  const char *dir = ::dirname(req);
  if( -1 == stat(dir, &sb) ) {
    if( -1 == mkdir(dir, 0700) ) {
      syslog( LOG_ERR, "%d: '%s': %m", __LINE__, dir );
      return EXIT_FAILURE;
    }
  }

  if( -1 == chdir(dir) ) {
    syslog( LOG_ERR, "could not change to '%s' (based on  %s), %d: %m", 
	    requests, dir, __LINE__ );
    return EXIT_FAILURE;
  }
    
  for( int fd=0; fd < 3; fd++ ) {
    static const char dev_null[] = "/dev/null";
    if( -1 == close(fd) ) {
      syslog( LOG_ERR, "%d: %m", __LINE__);
      return EXIT_FAILURE;
    }
    
    if( -1 == open(dev_null, 0, 0) ) {
      syslog( LOG_ERR, "%d: %m", __LINE__);
      return EXIT_FAILURE;
    }
  }    

  /*
   * Create request directory and FIFO if necessary, and open it.
   */
  if( -1 == stat(requests, &sb) ) {
    if( -1 == mkfifo(requests, 0640) ) {
      syslog( LOG_ERR, "%d: '%s': %m", __LINE__, requests);
      return EXIT_FAILURE;
    }
  }

  if( -1 == pidfile(name) ) {
    syslog( LOG_ERR, "%d: pidfile '%s': %m", __LINE__, name );
    char *user = getenv("USER");
    if( user && string("jklowden") == user )
      syslog( LOG_INFO, 
	      "%d: continuing without pidfile", __LINE__ );
    else
      return EXIT_FAILURE;
  } else {
    if( -1 == atexit(unlink_pidfile) ) {
      syslog( LOG_ERR, "%d: atexit(3) failed: %m", __LINE__ );
    }
  }

  syslog( LOG_INFO, "%d: reading requests on '%s'", __LINE__, ::requests );

  ifstream requests(::requests);
  if( ! requests.is_open() ) {
    syslog( LOG_ERR, "%d: failed to open ifstream on %s", 
	    __LINE__, ::requests );
    return EXIT_FAILURE;
  }

  while( requests >> wal_desc ) {
    // find or create response pipe
    pidmap::iterator pout = results.find(wal_desc.pid);
    int output;
    if( pout == results.end() ) {
      ostringstream name;
      name << responses << '/' << pid;
      if( -1 == stat(name.str().c_str(), &sb) ) {
	/*
	 * Response FIFO name *must* exist, else it's a logic error. 
	 * Caller is responsible to set up FIFO and have read pending
	 * on it.
	 */
	syslog( LOG_ERR, "%d: %s: %m", __LINE__, name.str().c_str() );
	return EXIT_FAILURE;
      }
      if( (output = open(name.str().c_str(), O_WRONLY, O_NONBLOCK)) == -1 ) {
	syslog( LOG_ERR, "%d: %m", __LINE__);
	return EXIT_FAILURE;
      }
      results[wal_desc.pid] = output;
    } else {
      output = pout->second;
    }


    // initialize WAL and send errno back to caller
    try {
      WalFile walfile( dirname, wal_desc );
    } 
    catch( const std::exception& oops ) {
      syslog( LOG_ERR, "%d: %s", __LINE__, oops.what() );
      return EXIT_FAILURE;
    }

  }
}

