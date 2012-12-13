#pragma once
#include <string>

// Current file format (subject to change...)
// Every entry is followed by an MD4 checksum to check integrity
// Updating the header MD4 After modifications is required (setting Complete flag). 
// All other operations should be append only? (Or truncate in the case of errors?)

//Non-overlapping transactions
//Log Header
// 8 - Signature
// 4 - Version
// 8 - LSN
// 8 - Timestamp
// 4 - Salt
// 4 - Header Size
// 4 - Complete
// 4 - Size
// 980 - (reserved -- padded with zeros)
// 16 - MD4

//Starting at offset 1024
//Transaction Begin
// 4 - Type
// 8 - Transaction ID
// 8 - Timestamp
// 8 - Size (Number of column/revision pairs)
// | 8 - Column ID
// | 8 - Revision
// 16 - MD4

//Transaction End
// 4 - Type
// 8 - Transaction ID
// 8 - Timestamp
// 16 - MD4

//Revision
// 4 - Type
// 8 - Column ID
// 8 - Revision
// 8 - Timestamp
// 4 - Size
// .. Data
// 16 - MD4

//Checkpoint
// 4 - Type
// 8 - Column ID
// 8 - Revision
// 8 - Timestamp
// 16 - MD4

//Rollback
// 4 - Type
// 8 - Transaction ID
// 8 - Timestamp
// 16 - MD4

const static int LogHeaderSize = 1024;
const static int Version = 0;
const static char* Signature = "Fastore\0";
const static int MaxLogSize = 104857600;
const static char* LogExtension = "fastlog\0";

//These represent the Non-data size of records. We should be able to compute if
//a record will fit by taking the size of the header, plus the size of the data we want to
//stick into a record.
//TODO: Consider fixed-size record headers.
const static int TransactionBeginSize = 48;
const static int TransactionEndSize = 26;
const static int RevisionSize = 48;
const static int CheckpointSize = 44;
const static int RollbackSize = 36;


