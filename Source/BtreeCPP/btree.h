#pragma once

#include <iostream>
#include <string>
#include <functional>
//#include <BTreeObserver.h>

using namespace std;

template <class K, class V>
class Split;

template <class K, class V>
class Leaf;

// TODO: temp due to missing file
template <class K, class V>
class IObserver
{
	public:
		inline virtual void ValuesMoved(Leaf<K, V>* leaf);
};

template <class K, class V>
class InsertResult
{
	public:
		V found;
		Split<K, V>* split;
};

template <class K, class V>
class INode
{
	public:
		virtual ~INode() {}
		virtual InsertResult<K, V> Insert(K key, V value, Leaf<K, V>* leaf) = 0;
		virtual wstring ToString() = 0;
};

template <class K, class V>
class Split
{
	public:
		K key;
		INode<K, V>* right;
};

template <class K, class V>
class BTree
{
	public:
		int Fanout;
		int LeafSize;
		int (*Compare)(K left, K right);
		wstring (*KeyToString)(K item);
		wstring (*ValueToString)(V item);
		IObserver<K, V>* Observer;
	
		BTree(int fanout, int leafSize, int(*compare)(K, K), wstring(*keyToString)(K), wstring(*valueToString)(V));

		V Insert(K key, V value, Leaf<K, V>* leaf);
		wstring ToString();

	private:
		INode<K, V>* _root;
		void DoValuesMoved(Leaf<K, V>*);

	template <class K, class V> friend class Leaf;
	template <class K, class V> friend class Branch;
};

template <class K, class V>
class Leaf: public INode<K, V>
{
	public:
		Leaf(BTree<K, V>* tree);

		InsertResult<K, V> Insert(K key, V value, Leaf<K, V>* leaf);	
		K GetKey(function<bool(V)>);
		wstring ToString();

	private:
		int Count;
		BTree<K, V>* _tree;
		K* _keys;
		V* _values;

		int IndexOf(K key);
		V InternalInsert(int index, K key, V value, Leaf<K, V>* leaf);
};

template <class K, class V>
class Branch : public INode<K, V>
{
	public:
		Branch(BTree<K, V>* tree);
		Branch(BTree<K, V>* tree, INode<K, V>* left, INode<K, V>* right, K key);

		InsertResult<K, V> Insert(K key, V value, Leaf<K, V>* leaf);
		wstring ToString();		

	private:
		int Count;
		BTree<K, V>* _tree;
		K* _keys;
		INode<K, V>** _children;

		int IndexOf(K key);
		Split<K, V>* InsertChild(int index, K key, INode<K, V>* child);
		void InternalInsertChild(int index, K key, INode<K, V>* child);
};

/* Template implementations */

//Tree
template <class K, class V>
inline BTree<K, V>::BTree(int fanout, int leafSize, int (*compare)(K, K), wstring(*keyToString)(K), wstring(*valueToString)(V))
{
	Compare = compare;
	KeyToString = keyToString;
	ValueToString = valueToString;
	Fanout = fanout;
	Observer = NULL;
	LeafSize = leafSize;
	_root = new Leaf<K, V>(this);
}

template <class K, class V>
inline V BTree<K, V>::Insert(K key, V value, Leaf<K, V>* leaf)
{
	InsertResult<K, V> result = _root->Insert(key, value, leaf);
	if (result.split != NULL)
	{
		_root = new Branch<K, V>(this, _root, result.split->right, result.split->key);
		delete(result.split);
	}

	return result.found;
}

template <class K, class V>
inline void BTree<K, V>::DoValuesMoved(Leaf<K, V>* leaf)
{
	if (Observer != NULL)
		Observer->ValuesMoved(leaf);
}

template <class K, class V>
inline wstring BTree<K, V>::ToString()
{
	return _root->ToString();
}

//Branch
template <class K, class V>
inline Branch<K, V>::Branch(BTree<K, V>* tree)
{
	_tree = tree;
	_children = new INode<K, V>*[_tree->Fanout];
	_keys = new K[_tree->Fanout - 1];
	Count = 0;
}

template <class K, class V>
inline Branch<K, V>::Branch(BTree<K, V>* tree, INode<K, V>* left, INode<K, V>* right, K key)
{
	_tree = tree;
	_children = new INode<K, V>*[_tree->Fanout];
	_keys = new K[_tree->Fanout - 1];
	_children[0] = left;
	_children[1] = right;
	_keys[0] = key;
	Count = 1;
}

template <class K, class V>
inline InsertResult<K, V> Branch<K, V>::Insert(K key, V value, Leaf<K, V>* leaf)
{
	int index = IndexOf(key);
	InsertResult<K, V> result = _children[index]->Insert(key, value, leaf);

	if (result.split != NULL)
	{
		Split<K, V>* temp = result.split;
		result.split = InsertChild(index, result.split->key, result.split->right);
		delete(temp);
	}
	return result;
}

template <class K, class V>
inline Split<K, V>* Branch<K, V>::InsertChild(int index, K key, INode<K, V>* child)
{
	if (Count != _tree->Fanout - 1)
	{
		InternalInsertChild(index, key, child);
		return NULL;
	}
	else
	{
		int mid = (Count + 1) / 2;
		Branch<K, V>* node = new Branch<K, V>(_tree);
		node->Count = Count - mid;
		memcpy(&node->_keys[0], &_keys[mid], node->Count * sizeof(K));
		memcpy(&node->_children[0], &_children[mid], (node->Count + 1) * sizeof(INode<K, V>*));

		Count = mid - 1;

		Split<K, V>* split = new Split<K, V>();
		split->key = _keys[mid - 1];
		split->right = node;

		if (index <= Count)
			InternalInsertChild(index, key, child);
		else
			node->InternalInsertChild(index - (Count + 1), key, child);

		return split;
	}
}


template <class K, class V>
inline void Branch<K, V>::InternalInsertChild(int index, K key, INode<K, V>* child)
{
	int size = Count - index;
	if (Count != index)
	{
		memmove(&_keys[index + 1], &_keys[index],  size * sizeof(K));
		memmove(&_children[index + 2], &_children[index + 1],  size * sizeof(INode<K, V>*));
	}

	_keys[index] = key;
	_children[index + 1] = child;
	Count++;
}


template <class K, class V>
inline int Branch<K, V>::IndexOf(K key)
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

template <class K, class V>
inline wstring Branch<K, V>::ToString()
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
template <class K, class V>
inline Leaf<K, V>::Leaf(BTree<K, V>* tree)
{
	_tree = tree;
	_keys = new K[_tree->LeafSize];
	_values = new V[_tree->LeafSize];
	Count = 0;
}

template <class K, class V>
inline InsertResult<K, V> Leaf<K, V>::Insert(K key, V value, Leaf<K, V>* leaf)
{
	int index = IndexOf(key);
	InsertResult<K, V> result;
	if (Count != _tree->LeafSize)
	{
		result.found = InternalInsert(index, key, value, leaf);
		result.split = NULL;
	}
	else
	{
		Leaf<K, V>* node = new Leaf<K, V>(_tree);
		if (index != Count)
		{
			node->Count = (_tree->LeafSize + 1) / 2;
			Count = Count - node->Count;

			memcpy(&node->_keys[0], &_keys[node->Count],  node->Count * sizeof(K));
			memcpy(&node->_values[0], &_values[node->Count], node->Count * sizeof(V));
		}

		if (index < Count)
			result.found = InternalInsert(index, key, value, leaf);
		else
			result.found = node->InternalInsert(index - Count, key, value, leaf);

		_tree->DoValuesMoved(node);

		Split<K, V>* split = new Split<K, V>();
		split->key = node->_keys[0];
		split->right = node;

		result.split = split;

		return result;
	}


	return result;
}

template <class K, class V>
inline V Leaf<K, V>::InternalInsert(int index, K key, V value, Leaf<K, V>* leaf)
{
	leaf = this;
	if (index >= Count || _tree->Compare(_keys[index], key) != 0)
	{
		if (Count != index)
		{
			memmove(&_keys[index + 1], &_keys[index], (Count - index) * sizeof(K));
			memmove(&_values[index + 1], &_values[index], (Count - index)  * sizeof(V));
		}
		_keys[index] = key;
		_values[index] = value;
		Count++;
		return NULL;
	}
	else
		return _values[index];
}

template <class K, class V>
inline int Leaf<K, V>::IndexOf(K key)
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

template <class K, class V>
inline wstring Leaf<K, V>::ToString()
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

		result << _tree->KeyToString(_keys[i]);
		result << ":";
		result << _tree->ValueToString(_values[i]);
	}
	result << "}";

	return result.str();
}

template <class K, class V>
inline K Leaf<K, V>::GetKey(function<bool(V)> predicate)
{
	for (int i = 0; i < Count; i++)
	{
		if (predicate(_values[i]))
			return _keys[i];
	}

	return NULL;
}
