#include "InternalTypes.h"
#include <hash_set>
#include "..\KeyTree.h"
#include "..\BTree.h"

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

template<> void CopyToArray<KeyTree*>(const void* item, void* arrpointer)
{
	memcpy(arrpointer, item, sizeof(KeyTree*));
}

template<> void CopyToArray<BTree*>(const void* item, void* arrpointer)
{
	memcpy(arrpointer, item, sizeof(BTree*));
}

std::wstring KeyTreeString(const void* item)
{
	wstringstream result;
	result << (*(KeyTree**)item)->ToString();
	return result.str();
}

void DeallocateKeyTree(void* items, int count)
{
	for (int i = 0; i < count; i++)
		(*(KeyTree**)items)[i].~KeyTree();
}

void DeallocateBTree(void* items, int count)
{
	for (int i = 0; i < count; i++)
		(*(BTree**)items)[i].~BTree();
}

KeyTreeType::KeyTreeType()
{
	CopyIn = CopyToArray<KeyTree*>;
	Size = sizeof(KeyTree*);
	ToString = KeyTreeString;
	Name = "KeyTreeType";
	Deallocate = DeallocateKeyTree;
}

BTreeType::BTreeType()
{
	CopyIn = CopyToArray<BTree*>;
	Size = sizeof(BTree*);
	Name = "BTreeType";
	Deallocate = DeallocateBTree;
}
