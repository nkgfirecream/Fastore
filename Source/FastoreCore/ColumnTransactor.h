#include "typedefs.h"
#include "Change.h"
#include "Range.h"

class ColumnTransactor
{
	public:
		virtual ValueKeyVector Get(Range range) = 0;
		virtual void Apply(TransactionID id, ChangeSet changes) = 0;
		virtual void Commit(TransactionID id) = 0;
		virtual ValueVector GetValues(TransactionID id, KeyVector keys) = 0; 
		virtual ValueKeyVectorVector GetSorted(TransactionID id, KeyVectorVector keyvector) = 0;
};
