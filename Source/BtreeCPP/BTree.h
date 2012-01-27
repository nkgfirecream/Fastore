#pragma once

#include <iostream>
#include <string>
#include <functional>
#include "optional.h"
#include "BTreeObserver.h"

using namespace std;

class Split;
class Leaf;

class InsertResult
{
	public:
		//optional<V> found;
		void* found;
		Split* split;
};

class INode
{
	public:
		virtual ~INode() {}
		virtual InsertResult Insert(void* key, void* value, Leaf* leaf) = 0;
		virtual wstring ToString() = 0;
};

class Split
{
	public:
		void* key;
		INode* right;
};

class BTree
{
	public:
		int Fanout;
		int LeafSize;
		int KeySize;
		int ValueSize;
		int (*Compare)(void* left, void* right);
		wstring (*KeyToString)(void* item);
		wstring (*ValueToString)(void* item);
		IObserver* Observer;
	
		BTree(int fanout, int leafSize, int keySize, int valueSize, int(*compare)(void*, void*), wstring(*keyToString)(void*), wstring(*valueToString)(void*));

		//optional<V> Insert(K key, V value, Leaf* leaf);
		void* Insert(void* key, void* value, Leaf* leaf);
		wstring ToString();

	private:
		INode* _root;
		void DoValuesMoved(Leaf*);

	 friend class Leaf;
	 friend class Branch;
};

class Leaf: public INode
{
	public:
		Leaf(BTree* tree);

		InsertResult Insert(void* key, void* value, Leaf* leaf);	
		void* GetKey(function<bool(void*)>);
		wstring ToString();

	private:
		int Count;
		BTree* _tree;
		byte* _keys;
		byte* _values;

		int IndexOf(void* key);
		//optional<V> InternalInsert(int index, K key, V value, Leaf* leaf);
		void* InternalInsert(int index, void* key, void* value, Leaf* leaf);
};

class Branch : public INode
{
	public:
		Branch(BTree* tree);
		Branch(BTree* tree, INode* left, INode* right, void* key);

		InsertResult Insert(void* key, void* value, Leaf* leaf);
		wstring ToString();		

	private:
		int Count;
		BTree* _tree;
		byte* _keys;
		INode** _children;

		int IndexOf(void* key);
		Split* InsertChild(int index, void* key, INode* child);
		void InternalInsertChild(int index, void* key, INode* child);
};

/* Template implementations */

//Tree
inline BTree::BTree(int fanout, int leafSize, int keySize, int valueSize, int (*compare)(void*, void*), wstring(*keyToString)(void*), wstring(*valueToString)(void*))
{
	Compare = compare;
	KeyToString = keyToString;
	ValueToString = valueToString;
	KeySize = keySize;
	ValueSize = valueSize;
	Fanout = fanout;
	Observer = NULL;
	LeafSize = leafSize;
	_root = new Leaf(this);
}

//optional<V>
inline void* BTree::Insert(void* key, void* value, Leaf* leaf)
{
	InsertResult result = _root->Insert(key, value, leaf);
	if (result.split != NULL)
	{
		_root = new Branch(this, _root, result.split->right, result.split->key);
		delete(result.split);
	}

	return result.found;
}

inline void BTree::DoValuesMoved(Leaf* leaf)
{
	if (Observer != NULL)
		Observer->ValuesMoved(leaf);
}

inline wstring BTree::ToString()
{
	return _root->ToString();
}

//Branch
inline Branch::Branch(BTree* tree)
{
	_tree = tree;
	_children = new INode*[_tree->Fanout];
	_keys = new byte[(_tree->Fanout - 1) * _tree->KeySize];
	Count = 0;
}

inline Branch::Branch(BTree* tree, INode* left, INode* right, void* key)
{
	_tree = tree;
	_children = new INode*[_tree->Fanout];
	_keys = new  byte[(_tree->Fanout - 1) * _tree->KeySize];
	_children[0] = left;
	_children[1] = right;
	memcpy(&_keys[0], key, _tree->KeySize);
	Count = 1;
}

inline InsertResult Branch::Insert(void* key, void* value, Leaf* leaf)
{
	int index = IndexOf(key);
	InsertResult result = _children[index]->Insert(key, value, leaf);

	if (result.split != NULL)
	{
		Split* temp = result.split;
		result.split = InsertChild(index, result.split->key, result.split->right);
		delete(temp);
	}
	return result;
}

inline Split* Branch::InsertChild(int index, void* key, INode* child)
{
	if (Count != _tree->Fanout - 1)
	{
		InternalInsertChild(index, key, child);
		return NULL;
	}
	else
	{
		int mid = (Count + 1) / 2;
		Branch* node = new Branch(_tree);
		node->Count = Count - mid;
		memcpy(&node->_keys[0], &_keys[mid * _tree->KeySize], node->Count * _tree->KeySize);
		memcpy(&node->_children[0], &_children[mid], (node->Count + 1) * sizeof(INode*));

		Count = mid - 1;

		Split* split = new Split();
		split->key = &_keys[(mid - 1) * _tree->KeySize];
		split->right = node;

		if (index <= Count)
			InternalInsertChild(index, key, child);
		else
			node->InternalInsertChild(index - (Count + 1), key, child);

		return split;
	}
}

inline void Branch::InternalInsertChild(int index, void* key, INode* child)
{
	int size = Count - index;
	if (Count != index)
	{
		memmove(&_keys[(index + 1) *_tree->KeySize], &_keys[index * _tree->KeySize],  size * _tree->KeySize);
		memmove(&_children[index + 2], &_children[index + 1],  size * sizeof(INode*));
	}

	memcpy(&_keys[index * _tree->KeySize], key, _tree->KeySize);
	_children[index + 1] = child;
	Count++;
}

inline int Branch::IndexOf(void* key)
{	
    int lo = 0;
	int hi = Count - 1;
	int localIndex = 0;
	int result = -1;

	while (lo <= hi)
	{
		localIndex = (lo + hi) / 2;
        result = _tree->Compare(key, &_keys[localIndex * _tree->KeySize]);

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

inline wstring Branch::ToString()
{
	wstringstream result;

	result << "\n[";
	bool first = true;
	for (int i = 0; i <= Count; i++)
	{
		if (!first)
			result << ",";
		else
			first = false;

		result << i << ": " << _children[i]->ToString();
	}
	result << "]";

	return result.str();
}

//Leaf
inline Leaf::Leaf(BTree* tree)
{
	_tree = tree;
	_keys = new byte[tree->LeafSize * _tree->KeySize];
	_values = new byte[tree->LeafSize * _tree->ValueSize];
	Count = 0;
}

inline InsertResult Leaf::Insert(void* key, void* value, Leaf* leaf)
{
	int index = IndexOf(key);
	InsertResult result;
	if (Count != _tree->LeafSize)
	{
		result.found = InternalInsert(index, key, value, leaf);
		result.split = NULL;
	}
	else
	{
		Leaf* node = new Leaf(_tree);
		if (index != Count)
		{
			node->Count = (_tree->LeafSize + 1) / 2;
			Count = Count - node->Count;

			memcpy(&node->_keys[0], &_keys[node->Count * _tree->KeySize],  node->Count * _tree->KeySize);
			memcpy(&node->_values[0], &_values[node->Count * _tree->ValueSize], node->Count * _tree->ValueSize);
		}

		if (index < Count)
			result.found = InternalInsert(index, key, value, leaf);
		else
			result.found = node->InternalInsert(index - Count, key, value, leaf);

		_tree->DoValuesMoved(node);

		Split* split = new Split();
		split->key = &node->_keys[0];
		split->right = node;

		result.split = split;

		return result;
	}


	return result;
}

inline void* Leaf::InternalInsert(int index, void* key, void* value, Leaf* leaf)
{
	leaf = this;
	if (index >= Count || _tree->Compare(&_keys[index * _tree->KeySize], key) != 0)
	{
		if (Count != index)
		{
			memmove(&_keys[(index + 1) * _tree->KeySize], &_keys[index], (Count - index) * _tree->KeySize);
			memmove(&_values[(index + 1) * _tree->ValueSize], &_values[index], (Count - index) * _tree->ValueSize);
		}

		memcpy(&_keys[index * _tree->KeySize], key, _tree->KeySize);
		memcpy(&_values[index * _tree->ValueSize], value, _tree->ValueSize);
		Count++;
		return NULL;
	}
	else
		return &_values[index * _tree->KeySize];
}

inline int Leaf::IndexOf(void* key)
{
	int lo = 0;
	int hi = Count - 1;
	int localIndex = 0;
	int result = -1;

	while (lo <= hi)
	{
		localIndex = (lo + hi) / 2;
        result = _tree->Compare(key, &_keys[localIndex * _tree->KeySize]);

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

inline wstring Leaf::ToString()
{
	wstringstream result;
	result << "\n{";
	bool first = true;
	for(int i = 0; i < Count; i++)
	{
		if(!first)
			result << ",";
		else
			first = false;

		result << _tree->KeyToString(&_keys[i * _tree->KeySize]);
		result << ":";
		result << _tree->ValueToString(&_values[i * _tree->ValueSize]);
	}
	result << "}";

	return result.str();
}

inline void* Leaf::GetKey(function<bool(void*)> predicate)
{
	for (int i = 0; i < Count; i++)
	{
		if (predicate(&_values[i * _tree->ValueSize]))
			return &_keys[i * _tree->KeySize];
	}

	return NULL;
}