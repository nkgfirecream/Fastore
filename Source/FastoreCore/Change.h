#include "typedefs.h"


struct Change
{
	Key RowID;
	Value Value;
	enum OperationType {Include = 0, Exclude = 1};	
	OperationType Operation;
};

typedef eastl::vector<Change> ChangeSet;