#include "typedefs.h"
#include "Change.h"
#include "TransactionID.h"

class TransactionManager
{
	public:
		virtual TransactionID Start() = 0;
		virtual short Prepare(ChangeSet) = 0;
		virtual void Commit(TransactionID) = 0;
};
