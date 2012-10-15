#include "Wal.h"
// $Id: Wal.cpp 657 2012-10-15 17:19:55Z jklowden $

#include <sys/types.h>
///include <openssl/md4.h>

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
#define THROW(msg,value) {							\
		ostringstream oops;							\
		oops << __FILE__ << ":" << __LINE__			\
			 << ": error " << ": "					\
			 <<  msg << " '" << value << "': ";		\
		Log << log_err << oops.str() << log_endl;	\
		throw std::runtime_error( oops.str() );		\
	}

#define THROW_ERRNO(msg,value) {					\
		ostringstream oops;							\
		oops << __FILE__ << ":" << __LINE__			\
			 << ": error " << errno << ": "			\
			 <<  msg << " '" << value << "': "		\
			 << strerror(errno);					\
		Log << log_err << oops.str() << log_endl;	\
		throw std::runtime_error( oops.str() );		\
	}

const char *random_dev = "/dev/random";

void 
Wal::
randomness(int n, void *buf)
{
	int retries = 5;

	while( retries && read(random_fd, buf, n) != n ) {
		if( retries-- ) {
			sleep(1);
			continue;
		}
		THROW_ERRNO("could not use random-number generator", random_dev );
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
  // Does nothing until ported to windows. 
}  

Wal::Status 
Wal::apply(Revision revision, const ColumnWrites& writes)
{
	return OK;
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

