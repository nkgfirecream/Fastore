// $Id$
#pragma once

#include "../../FastoreCommunication/Comm_types.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#else
# include <io.h>
#endif

#if HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif
#include <sys/stat.h>

#include <string>

#include "../Log/Syslog.h"

///namespace fastore { namespace communication {
using fastore::communication::ColumnID;
using fastore::communication::ColumnWrites;
using fastore::communication::NetworkAddress;
using fastore::communication::Revision;
using fastore::communication::TransactionID;
using fastore::communication::Writes;

using fastore::Log;
using fastore::log_endl;
using fastore::log_info;
using fastore::log_debug;
using fastore::log_err;

#if DEBUG_WAL_FUNC
# define DEBUG_FUNC() {													\
		Log << log_debug << __FUNCTION__  << ":" << __LINE__  << log_endl; }
#else
# define DEBUG_FUNC() {}
#endif

struct ColumnRevisionRange
{
	ColumnID columnID;
	Revision first, last;

	ColumnRevisionRange( ColumnID columnID=0, Revision first=0, Revision last=0 )
		: columnID(columnID), first(first), last(last) {}
};

typedef std::set<ColumnRevisionRange> ColumnRevisionSet;

class Wal
{
public:
	enum Status { OK, Blocked, Error }; 

private:  std::string fileName;
	enum { WAL_FILE_SIZE = 1L << 30 };

	std::map< ColumnID, Revision > oldestRevisions;

	int os_error;
	Status status;

	std::string dirName, name;
	NetworkAddress addr;
	unsigned char *wal, *current;
	static int random_fd;

public:
	Wal( const std::string& dirName, 
		 const std::string& name, 
		 const NetworkAddress& addr );
  
	Status apply(Revision revision, const ColumnWrites& writes);

	Status Write( const TransactionID& transactionID, const Writes& writes );

	Status Flush( const TransactionID& transactionID );

	Writes GetChanges( const ColumnRevisionSet& col_revisions );

	const ColumnRevisionSet& Recover( const TransactionID& transactionID, 
									  ColumnRevisionSet& revisionSet /* OUT */ );

	int osError() const { return os_error; }
	static void randomness(int N, void *P);

private:
	void init(unsigned char *wal);
	bool verify();
	static unsigned char * verify_page( const unsigned char *data, size_t length );
	unsigned char * find_tail() const;
};
