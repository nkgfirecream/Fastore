// $Id$
#pragma once

#include <string>
#include <TransactionID.h>
#include <Comm_types.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

///namespace fastore { namespace communication {
using fastore::communication::ColumnID;
using fastore::communication::ColumnWrites;
using fastore::communication::NetworkAddress;
using fastore::communication::Revision;
//ing fastore::communication::TransactionID;
using fastore::communication::Writes;

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
  char *wal, *current;

public:
  Wal( const std::string& dirName, const std::string& name, 
       const NetworkAddress& addr );
  
  Status Write( const TransactionID& transactionID, const Writes& writes );

  Status Flush( const TransactionID& transactionID );

  Writes GetChanges( const ColumnRevisionSet& col_revisions );

  const ColumnRevisionSet& Recover( const TransactionID& transactionID, 
				    ColumnRevisionSet& revisionSet /* OUT */ );

  int osError() const { return os_error; }

private:
  void init(char *wal);
  bool verify();
};
