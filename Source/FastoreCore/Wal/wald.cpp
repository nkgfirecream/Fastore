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

using namespace std;

struct wal_desc_t
{
  const char prefix[8];
  const char suffix[5];
  char log_number[8];
  pid_t pid;

  string name() const {
    ostringstream os;
      
    os << prefix << '.' << log_number << suffix;
    return os.str();
  }
} wal_desc = { "FASTORE", ".LOG", "" };

istream&
operator>>( istream& is, wal_desc_t& desc ) 
{
  is >> desc.log_number >> desc.pid;
  return is;
}

static const char requests[] = "/var/wald/request";
static const char responses[] = "/var/wald/";

typedef std::map<pid_t, int> pidmap;
pidmap results;

int
main( int argc, char *argv[] )
{
  struct stat sb;

  // attach to syslogd
  char *name = basename(argv[0]);
  openlog( name, LOG_CONS, LOG_DAEMON);

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

  if( (pid = setsid()) == -1 ) {
    syslog( LOG_ERR, "%d: %m", __LINE__);
    return EXIT_FAILURE;
  }

  umask(0);

  char req[ sizeof(::requests) ];
  strcpy(req, ::requests);
  const char *dir = dirname(req);
  if( -1 == stat(dir, &sb) ) {
    if( -1 == mkdir(dir, 0700) ) {
    syslog( LOG_ERR, "%d: %m", __LINE__);
    return EXIT_FAILURE;
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
    syslog( LOG_ERR, "%d: %m", __LINE__);
    return EXIT_FAILURE;
  }
  
  // create the request FIFO unless it exists
  if( -1 == stat(requests, &sb) ) {
    if( -1 == mkfifo(requests, 0640) ) {
      syslog( LOG_ERR, "%d: %m", __LINE__);
      return EXIT_FAILURE;
    }
  }

  ifstream requests(::requests);
  if( ! requests.is_open() ) {
    syslog( LOG_ERR, "%d: failed to open ifstream on %s", 
	    __LINE__, ::requests );
    return EXIT_FAILURE;
  }

  if( -1 == pidfile(name) ) {
    syslog( LOG_ERR, "%d: %m", __LINE__);
    return EXIT_FAILURE;
  }

  while( requests >> wal_desc ) {
    // find or create response pipe
    pidmap::iterator pout = results.find(wal_desc.pid);
    int output;
    if( pout == results.end() ) {
      string name = wal_desc.name();
      if( -1 == stat(name.c_str(), &sb) ) {
	/*
	 * Response FIFO name *must* exist, else it's a logic error. 
	 * Caller is responsible to set up FIFO and have read pending
	 * on it.
	 */
	syslog( LOG_ERR, "%d: %s: %m", __LINE__, name.c_str() );
	return EXIT_FAILURE;
      }
      if( (output = open(name.c_str(), O_WRONLY, O_NONBLOCK)) == -1 ) {
	syslog( LOG_ERR, "%d: %m", __LINE__);
	return EXIT_FAILURE;
      }
      results[wal_desc.pid] = output;
    } else {
      output = pout->second;
    }


    // initialize WAL and send errno back to caller
    int error = initialize_wal_file(wal_desc);
  
    if( sizeof(error) != write(output, &error, sizeof(error)) ) {
      syslog( LOG_ERR, "%d: %m", __LINE__);
      return EXIT_FAILURE;
    }

  }

   
}

