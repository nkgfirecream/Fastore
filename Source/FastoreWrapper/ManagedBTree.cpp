#include "stdafx.h"
#include "ManagedBTree.h"

using namespace Wrapper;
using namespace System;


void ManagedBTree::Insert(int key, int value)
{
	pin_ptr<int> pinnedkey = &key;
	pin_ptr<int> pinnedvalue = &value;
	auto path = _nativeTree->GetPath((void *)pinnedkey);
	_nativeTree->Insert(path, (void*)pinnedkey, (void*)pinnedvalue);

}

void ManagedBTree::Delete(int key)
{
	pin_ptr<int> pinnedkey = &key;
	auto path = _nativeTree->GetPath((void *)pinnedkey);
	if(path.Match)
	{
		_nativeTree->Delete(path);
	}
}

int ManagedBTree::Get(int key)
{
	pin_ptr<int> pinnedkey = &key;
	auto path = _nativeTree->GetPath((void *)pinnedkey);
	if(path.Match)
	{
		return *(int*)(*path.Leaf)[path.LeafIndex].value;
	}
	else
	{
		return 0;
	}
}