#include "..\typedefs.h"
#include "..\Range.h"

class ColumnManager
{
	public:
		virtual ValueKeyVector Get(Range range) = 0;
		virtual void Apply(/* ChangeSet */) = 0;
		virtual void Commit(/* TID - Revision */) = 0;
		virtual ValueVector GetValues(KeyVector keys /* TID */) = 0; 
		virtual ValueKeyVectorVector GetSorted(KeyVectorVector keyvector /* TID */) = 0;
};
