#include "typedefs.h"
#include "Change.h"
#include "Range.h"
#include "TransactionID.h"

using namespace fs;
//Optimistic Concurrency Phases
//1. Start - Handled by our Mastertransction manager
//2. Modify - Read all old values and store them with our changes in a pending list
//3. Validate - Make sure all of our read values still match up
//4. Commit - Apply values to columns

//Validate requires a write lock on all items being validated. That lock must stay in place until Commit. The lock should be per row, reads and writes on other rows should be fine.
//Since we can't place locks on the B-Tree, we must have a write-locked list of rowIds (Btree?? of rowids - transactionids, if current transaction id matches the locked one,
//then allow the change, unlock the row. Btree itself needs to be protected. Concurrent structures?).

//Apply should never fail (in normal use)(it just reads whatever is present, and queues up the changes) (What happens if the apply was operating on the wrong revision?)

//Validate should only fail if changes are detected. Then there are two options for change detection: 
//1. Reread the data and compare it to the old data
//2. Check the transaction log and see if any transactions that committed after this one started changed our rowIDs. Option one seems simpler.
//Upon failure there are two options retry the apply by grabbing updated data (this would need to be done on the "table" level, huh? The column wouldn't know what to do with the apply)
//or telling the engine that the apply failed and letting it decide what to do.
// If ANY column fails validation, rollback. Otherwise, commit.

//Commit should never fail (in normal use). It just means apply the changes to the column buffer, write them to a log, and clear them from the pending list.
//Rollback should never fail (in normal use). It just means remove the changes from the pending log.

//Story for non-normal use such as hardware failure?
class ColumnTransactor
{
	public:
		virtual ValueKeysVector Get(Range range) = 0;
		virtual ValueVector GetValues(TransactionID id, KeyVector keys) = 0; 
		virtual ValueKeysVectorVector GetSorted(TransactionID id, KeyVectorVector keyvector) = 0;
		virtual void Apply(TransactionID id, ChangeSet changes) = 0;
		virtual bool Validate(TransactionID id) = 0;
		virtual void Commit(TransactionID id) = 0;		
		virtual void Rollback(TransactionID id) = 0;

		//Other structures..
		
		//Pending log - Filled during Apply - (Snapshot Nate was discussing?)
		//Log
		//	|-TransactionID
		//	|	|--- {OldValue, Change}
		//	|	|--- {OldValue, Change}
		//  |
		//	|-TransactionID
		//      |--- {OldValue, Change}

		// RowId - TransactionId
		// RowId - TransactionId		
};


//Would it be faster to combine apply and validate? We would have to lock each row as we went... 
//Apply - No locks (What about pending structure?)
// Insert TransactionId into pending log
// For each change, pull old value
// Store each change in log with old value

//Validate - 
// For each change in transaction's pending log
//	Lock RowID  -- If locked unlock all and wait (to prevent deadlock). If still locked after wait, fail.
//	Compare old value to stored old value
//	If different, fail

//Commit -
// For each change in transaction's pending log
//	Apply change to buffer
//	Log change
//	Unlock RowID

//Rollback
// For each change in transaction's pending log
//	Unlock RowID
//	Delete Change.