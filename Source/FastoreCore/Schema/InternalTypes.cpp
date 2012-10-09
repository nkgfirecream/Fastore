#include "InternalTypes.h"
#include <unordered_set>
#include "../BTree.h"

void DeallocateHashSet(void* items, int count)
{
	// TODO: How to call a unordered_set destructor?
	//for (int i = 0; i < count; i++)
	//	((std::unordered_set<void*, ScalarType, ScalarType>*)items)[i].~_Hash();
}

HashSetType::HashSetType()
{
	Name = "HashSet";
	Compare = NULL;
	Size = sizeof(std::unordered_set<void*, ScalarType, ScalarType>*);
	ToString = NULL;
	CopyIn = CopyToArray<std::unordered_set<void*, ScalarType, ScalarType>*>;
	Deallocate = DeallocateHashSet;
}


template<> void CopyToArray<BTree*>(const void* item, void* arrpointer)
{
	memcpy(arrpointer, item, sizeof(BTree*));
}

void DeallocateBTree(void* items, int count)
{
	for (int i = 0; i < count; i++)
	{
		delete ((BTree**)items)[i];
	}
}

BTreeType::BTreeType()
{
	CopyIn = CopyToArray<BTree*>;
	Size = sizeof(BTree*);
	Name = "BTreeType";
	Deallocate = DeallocateBTree;
}
