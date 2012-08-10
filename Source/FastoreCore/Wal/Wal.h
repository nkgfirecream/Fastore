// $Id$
#pragma once

#include <string>
#include <TransactionID.h>
#include <Comm_types.h>

///namespace fastore { namespace communication {
using fastore::communication::ColumnID;
using fastore::communication::ColumnWrites;
using fastore::communication::NetworkAddress;
using fastore::communication::Revision;
using fastore::communication::TransactionID;


class Change 
{
  const Revision revision;
  const ColumnID columnID;
  typedef std::list<ColumnWrites> Writes;
  Writes writes;

public:
  Change( Revision revision, ColumnID columnID ) 
    : revision(revision), columnID(columnID)
  {}

  Change& operator<<( const Writes& writes );

  Revision getRevision() const { return revision; }
  ColumnID getColumnID() const { return columnID; }

  const Writes& getWrites() const { return writes; }

  // permit sorting
  bool operator<( const Change& that ) const;
};

typedef  std::set<Change> ChangeSet;

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
  char *wal;

public:
  Wal( const std::string& dirName, const std::string& name, 
       const NetworkAddress& addr );
  
  Status Write( const TransactionID& transactionID, const ChangeSet& changes );
  Status Write( Revision revision, Writes writes );

  Status Flush( const TransactionID& transactionID );

  ChangeSet GetChanges( const ColumnRevisionSet& col_revisions );

  const ColumnRevisionSet& Recover( const TransactionID& transactionID, 
				    ColumnRevisionSet& revisionSet /* OUT */ );

  int osError() const { return os_error; }

private:
  void VerifyFile();

};
