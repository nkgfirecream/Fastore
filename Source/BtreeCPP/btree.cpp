#include <iostream>
#include <string>
#include <sstream>
#include "btree.h"
using namespace std;

//Tree
BTree::BTree(int fanout, int leafsize, int (*compare)(void*,void*), char*(*tostring)(void*))
{
	Compare = compare;
	ItemToString = tostring;
	Fanout = fanout;
	LeafSize = leafsize;
	_root = new Leaf(this);
}

void* BTree::Insert(void* key, void* value)
{
	InsertResult result = _root->Insert(key,value);
	if(result.split != NULL)
	{
		_root = new Branch(this, _root, result.split->right, result.split->key);
		delete(result.split);
	}

	return result.found;
}

string BTree::ToString()
{
	return _root->ToString();
}

//Branch
Branch::Branch(BTree* tree)
{
	_tree = tree;
	_children = new INode*[_tree->Fanout];
	_keys = new void*[_tree->Fanout - 1];
	Count = 0;
}

Branch::Branch(BTree* tree, INode* left, INode* right, void* key)
{
	_tree = tree;
	_children = new INode*[_tree->Fanout];
	_keys = new void*[_tree->Fanout - 1];
	_children[0] = left;
	_children[1] = right;
	_keys[0] = key;
	Count = 1;
}

InsertResult Branch::Insert(void* key, void* value)
{
	int index = IndexOf(key);
	InsertResult result = _children[index]->Insert(key, value);

	if(result.split != NULL)
	{
		Split* temp = result.split;
		result.split = InsertChild(index, result.split->key, result.split->right);
		delete(temp);
	}
	return result;
}

Split* Branch::InsertChild(int index, void* key, INode* child)
{
	if (Count == _tree->Fanout - 1)
	{
		int mid = (Count + 1) / 2;
		Branch* node = new Branch(_tree);
		node->Count = Count - mid;
		memcpy(&node->_keys[0], &_keys[mid], node->Count * sizeof(void*));
		memcpy(&node->_children[0], &_children[mid], (node->Count + 1) * sizeof(void*));

		Count = mid - 1;

		Split* split = new Split();
		split->key = _keys[mid - 1];
		split->right = node;

		if(index <= Count)
			InternalInsertChild(index, key, child);
		else
			node->InternalInsertChild(index - (Count + 1), key, child);

		return split;
	}
	else
	{
		InternalInsertChild(index, key, child);
		return NULL;
	}
}


void Branch::InternalInsertChild(int index, void* key, INode* child)
{
	int size = Count - index;
	memmove(&_keys[index + 1], &_keys[index],  size * sizeof(void*));
	memmove(&_children[index + 2], &_children[index + 1],  size * sizeof(void*));

	_keys[index] = key;
	_children[index + 1] = child;
	Count++;
}


int Branch::IndexOf(void* key)
{	
        int lo = 0;
		int hi = Count - 1;
		int localIndex = 0;
		int result = -1;

		while (lo <= hi)
		{
			localIndex = (lo + hi) / 2;
            result = _tree->Compare(key, _keys[localIndex]);

			if (result == 0)
				break;
			else if (result < 0)
				hi = localIndex - 1;
			else
				lo = localIndex + 1;
		}

        if (result == 0)
            return localIndex;
        else
            return lo;
}

string Branch::ToString()
{
	stringstream result;

	result << "\n[";
	bool first = true;
	for(int i = 0; i <= Count; i++)
	{
		if(!first)
			result << ",";
		else
			first = false;

		result << i << ": " << _children[i]->ToString();
	}
	result << "]";

	return result.str();
}

//Leaf
Leaf::Leaf(BTree* tree)
{
	_tree = tree;
	_keys = new void*[_tree->LeafSize];
	_values = new void*[_tree->LeafSize];
	Count = 0;
}

InsertResult Leaf::Insert(void* key, void* value)
{
	int index = IndexOf(key);
	InsertResult result;
	if(Count == _tree->LeafSize)
	{
		Leaf* node = new Leaf(_tree);
		node->Count = (_tree->LeafSize + 1) / 2;
		Count = Count - node->Count;

		memcpy(&node->_keys[0], &_keys[node->Count],  node->Count * sizeof(void*));
		memcpy(&node->_values[0], &_values[node->Count], node->Count * sizeof(void*));

		if(index < Count)
		{
			result.found = InternalInsert(index, key, value);
		}
		else
		{
			result.found = node->InternalInsert(index - Count, key, value);
		}

		Split* split = new Split();
		split->key = node->_keys[0];
		split->right = node;

		result.split = split;

		return result;
	}
	else
	{
		result.found = InternalInsert(index, key, value);
		result.split = NULL;
	}

	return result;
}

void* Leaf::InternalInsert(int index, void* key, void* value)
{
	if(index < Count && _tree->Compare(_keys[index], key) == 0)
	{
		return _values[index];
	}
	else
	{
		memmove(&_keys[index + 1], &_keys[index], (Count - index) * sizeof(void*));
		memmove(&_values[index + 1], &_values[index], (Count - index)  * sizeof(void*));
		_keys[index] = key;
		_values[index] = value;
		Count++;
		return NULL;
	}
}

int Leaf::IndexOf(void* key)
{
	int lo = 0;
	int hi = Count - 1;
	int localIndex = 0;
	int result = -1;

	while (lo <= hi)
	{
		localIndex = (lo + hi) / 2;
        result = _tree->Compare(key, _keys[localIndex]);

		if (result == 0)
			break;
		else if (result < 0)
			hi = localIndex - 1;
		else
			lo = localIndex + 1;
	}

    if (result == 0)
        return localIndex;
    else
        return lo;
}

string Leaf::ToString()
{
	string result = "\n{";
	bool first = true;
	for(int i = 0; i < Count; i++)
	{
		if(!first)
			result += ",";
		else
			first = false;

		result += _tree->ItemToString(_keys[i]);
		result += ":";
		result += _tree->ItemToString(_values[i]);
	}
	result += "}";

	return result;
}
		









