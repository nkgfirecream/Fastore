#include "InternalTypes.h"
#include <hash_set>
#include "../BTree.h"

void DeallocateHashSet(void* items, int count)
{
	// TODO: How to call a hash_set destructor?
	//for (int i = 0; i < count; i++)
	//	((std::hash_set<void*, ScalarType, ScalarType>*)items)[i].~_Hash();
}

HashSetType::HashSetType()
{
	Name = "HashSet";
	Compare = NULL;
	Size = sizeof(std::hash_set<void*, ScalarType, ScalarType>*);
	ToString = NULL;
	CopyIn = CopyToArray<std::hash_set<void*, ScalarType, ScalarType>*>;
	Deallocate = DeallocateHashSet;
}


template<> void CopyToArray<BTree*>(const void* item, void* arrpointer)
{
	memcpy(arrpointer, item, sizeof(BTree*));
}

void DeallocateBTree(void* items, int count)
{
	for (int i = 0; i < count; i++)
		(*(BTree**)items)[i].~BTree();
}

BTreeType::BTreeType()
{
	CopyIn = CopyToArray<BTree*>;
	Size = sizeof(BTree*);
	Name = "BTreeType";
	Deallocate = DeallocateBTree;
}
