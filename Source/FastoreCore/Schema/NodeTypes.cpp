#include "NodeTypes.h"
#include "../BTree.h"
#include <cstring>

void DeallocateNode(void* items, int count)
{
	for (int i = 0; i < count; i++)
		delete ((Node**)items)[i];
}

template<> void CopyToArray<Node*>(const void* item, void* arrpointer)
{
	memcpy(arrpointer, item, sizeof(Node*));
}

NodeType::NodeType()
{
	CopyIn = CopyToArray<Node*>;
	Name = "NodeType";
	Size = sizeof(Node*);
	Deallocate = DeallocateNode;
}

NoOpNodeType::NoOpNodeType()
{
	CopyIn = CopyToArray<Node*>;
	Name = "NoOpNodeType";
	Size = sizeof(Node*);
	Deallocate = NoOpDeallocate;
}
