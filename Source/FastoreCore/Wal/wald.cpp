/*
 * $Id$
 * Allocate WAL files.
 * Accept requests over FIFO and respond over another.
 */

#include <err.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <string>

#include <iostream>
#include <fstream>
#include <sstream>

#include <map>

#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <syslog.h>

#include <sys/stat.h>

#include "WalFile.h"

using namespace std;

int pidfile(const char *basename);

wal_desc_t wal_desc;

istream&
operator>>( istream& is, wal_desc_t& desc ) 
{
	is >> desc.log_number >> desc.pid;
	return is;
}

typedef std::map<pid_t, int> pidmap;
pidmap results;

static void usage()
{
	cerr << "syntax: wald [-d logdir]\n"; 
}

static void unlink_pidfile(void)
{
	static const char name[] = "/var/run/wald";
	unlink(name);
	syslog( LOG_INFO, "terminated" );
}

int
main( int argc, char *argv[] )
{
	std::string logdir("/var/fastore");
	const char *name = basename(argv[0]);

	extern char *optarg;
	extern int optind;
	int bflag, ch, fd;

	bflag = 0;
	while ((ch = getopt(argc, argv, "d:")) != -1) {
		switch (ch) {
		case 'd':
			logdir = optarg;
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
	 */
	pid_t pid = fork();

	if( pid != 0 ) {
		if( pid == -1 ) {
			err(errno, "wald");
		}
		return EXIT_SUCCESS;
	}

	syslog( LOG_INFO, "%d: %s: started", __LINE__, name);

	if( (pid = setsid()) == -1 ) {
		syslog( LOG_ERR, "%d: %m", __LINE__);
		return EXIT_FAILURE;
	}

	umask(0);

	/*
	 * Create the request directory if needed, and chdir there. 
	 */
	char req[ sizeof(Wald::requests) ];
	strcpy(req, Wald::requests);
	const char *dir = ::dirname(req);
	if( -1 == stat(dir, &sb) ) {
		syslog( LOG_INFO, "%d: creating request directory '%s'", 
				__LINE__, Wald::responses );
		if( -1 == mkdir(dir, 0700) ) {
			syslog( LOG_ERR, "%d: '%s': %m", __LINE__, dir );
			return EXIT_FAILURE;
		}
	}

	syslog( LOG_INFO, "%d: changing to log-request directory %s", __LINE__, dir );
	if( -1 == chdir(dir) ) {
		syslog( LOG_ERR, "could not change to '%s' (based on  %s), %d: %m", 
				Wald::requests, dir, __LINE__ );
		return EXIT_FAILURE;
	}
    
	/*
	 * Close existing descriptor and reopen them on the null device.
	 */  
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
	 * Ignore SIGPIPE in case writing to a dead requestor.
	 * Otherwise we'd be killed. 
	 */
	if( SIG_ERR == signal(SIGPIPE, SIG_IGN) ) {
		syslog( LOG_ERR, "%d: cannot ignore SIGPIPE: %m", __LINE__);
		return EXIT_FAILURE;
	}

	/*
	 * Create request directory and FIFO if necessary, and open it.
	 */
	if( -1 == stat(Wald::requests, &sb) ) {
		syslog( LOG_INFO, "%d: creating request FIFO '%s'", 
				__LINE__, Wald::requests );
		if( -1 == mkfifo(Wald::requests, 0640) ) {
			syslog( LOG_ERR, "%d: '%s': %m", __LINE__, Wald::requests);
			return EXIT_FAILURE;
		}
	}

	/*
	 * Create response directory if necessary. 
	 */
	if( -1 == stat(Wald::responses, &sb) ) {
		syslog( LOG_INFO, "%d: creating response directory '%s'", 
				__LINE__, Wald::responses );
		if( -1 == mkdir(Wald::responses, 0750) ) {
			syslog( LOG_ERR, "%d: '%s': %m", __LINE__, Wald::responses);
			return EXIT_FAILURE;
		}
	}

	/*
	 * Create pid file if possible. 
	 */
	if( -1 == pidfile(name) ) {
		syslog( LOG_ERR, "%d: pidfile '%s': %m", __LINE__, name );
		char *user = getenv("USER");
		if( user && string("jklowden") == user )
			syslog( LOG_INFO, 
					"%d: continuing without pidfile", __LINE__ );
		else
			return EXIT_FAILURE;
	} 

	if( -1 == atexit(unlink_pidfile) ) {
		syslog( LOG_ERR, "%d: atexit(3) failed: %m", __LINE__ );
	}


	syslog( LOG_INFO, "%d: opening '%s'", __LINE__, Wald::requests );

	int requests;
	if( (requests = open(Wald::requests, O_RDONLY, 0)) == -1 ) {
		syslog( LOG_ERR, "%d: failed to open %s", __LINE__, Wald::requests );
		return EXIT_FAILURE;
	}

	size_t nread(0);
	while( nread == 0 && requests >= 0 ) {
		syslog( LOG_INFO, "%d: reading requests on '%s'", __LINE__, Wald::requests );
		while( (nread = read(requests, &wal_desc, sizeof(wal_desc)))
			   == sizeof(wal_desc) ) {
			syslog( LOG_INFO, "%d: request from %d", __LINE__, wal_desc.pid );
			// find or create response pipe
			pidmap::iterator pout = results.find(wal_desc.pid);
			int output;
			if( pout == results.end() ) {
				ostringstream name;
				name << Wald::responses << wal_desc.pid;
				if( -1 == stat(name.str().c_str(), &sb) ) {
					/*
					 * Response FIFO name *must* exist, else it's a logic error. 
					 * Caller is responsible to set up FIFO and have read pending
					 * on it. But failure to do so doesn't take down the daemon. 
					 */
					syslog( LOG_ERR, "%d: %s: %m", __LINE__, name.str().c_str() );
					continue;
				}
				if( (output = open(name.str().c_str(), O_WRONLY, 0)) == -1 ) {
					syslog( LOG_ERR, "%d: %m", __LINE__);
					return EXIT_FAILURE;
				}
				results[wal_desc.pid] = output;
			} else {
				output = pout->second;
			}

			// initialize WAL and send status back to caller
			int err;
			const string wal_name(logdir + "/" + wal_desc.name());
			try {
				WalFile walfile( logdir, wal_desc );
				syslog( LOG_INFO, "%d: generated %s", __LINE__, wal_name.c_str() );
				err = 0;
			} 
			catch( const std::exception& oops ) {
				syslog( LOG_ERR, "%d: %s", __LINE__, oops.what() );
				err = 1;
			}

			if( sizeof(err) != write(output, &err, sizeof(err)) ) {
				syslog( LOG_ERR, "%d: could not respond for %s: %m", 
						__LINE__, wal_name.c_str() );
			}
		}

		if( 0 == nread ) { // no requests outstanding; re-open and wait
			syslog( LOG_INFO, "%d: idle", __LINE__ );
			if( (requests = open(Wald::requests, O_RDONLY, 0)) == -1 ) {
				syslog( LOG_ERR, "%d: failed to open %s", __LINE__, Wald::requests );
				return EXIT_FAILURE;
			}
		}
	}
  
	syslog( LOG_ERR, "%d: error reading %s: %m", __LINE__, Wald::requests );

}

