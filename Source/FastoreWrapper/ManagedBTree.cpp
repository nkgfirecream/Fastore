#include "stdafx.h"
#include "ManagedBTree.h"

using namespace Wrapper;
using namespace System;


void ManagedBTree::Insert(int key, int value)
{
	//pin_ptr<int> pinnedkey = &key;
	//pin_ptr<int> pinnedvalue = &value;
	auto path = _nativeTree->GetPath(&key);
	_nativeTree->Insert(path, &key, &value);

}

void ManagedBTree::Delete(int key)
{
	//pin_ptr<int> pinnedkey = &key;
	auto path = _nativeTree->GetPath(&key);
	if(path.Match)
	{
		_nativeTree->Delete(path);
	}
}

int ManagedBTree::Get(int key)
{
	//pin_ptr<int> pinnedkey = &key;
	auto path = _nativeTree->GetPath(&key);
	if(path.Match)
	{
		return *(int*)(*path.Leaf)[path.LeafIndex].value;
	}
	else
	{
		return 0;
	}
}