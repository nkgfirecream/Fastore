#include "..\typedefs.h"

class TransactionManager
{
	public:
		virtual void /*TID*/ Start() = 0;
		virtual void /*REV*/ Prepare(/* ChangeSet */) = 0;
		virtual void Commit(/* TID*/) = 0;
};
