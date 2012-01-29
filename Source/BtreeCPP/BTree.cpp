#include "BTree.h"
#include <string>
#include <sstream>

using namespace std;

//Tree

BTree::BTree(type keyType, type valueType, IObserver* observer)
{
	_keyType = keyType;
	_valueType = valueType;
	_branchCapacity = 128;
	_leafCapacity = 128;
	_observer = observer;
	_root = new Leaf(this);
}

BTree::~BTree()
{
	delete _root;
}

void* BTree::Insert(void* key, void* value, Leaf** leaf)
{
	InsertResult result = _root->Insert(key, value, leaf);
	if (result.split != NULL)
	{
		_root = new Branch(this, _root, result.split->right, result.split->key);
		delete result.split;
	}

	return result.found;
}

// Set the branch and leaf capacities; only affects nodes that are created after this is set
void BTree::setCapacity(int branchCapacity, int leafCapacity)
{
	_branchCapacity = branchCapacity;
	_leafCapacity = leafCapacity;
}

int BTree::getBranchCapacity()
{
	return _branchCapacity;
}

int BTree::getLeafCapacity()
{
	return _leafCapacity;
}

void BTree::DoValuesMoved(Leaf* leaf)
{
	if (_observer != NULL)
		_observer->ValuesMoved(leaf);
}

wstring BTree::ToString()
{
	return _root->ToString();
}

//Branch

Branch::Branch(BTree* tree)
{
	_tree = tree;
	_children = new INode*[_tree->_branchCapacity];
	_keys = new char[(_tree->_branchCapacity - 1) * _tree->_keyType.Size];
	Count = 0;
}

Branch::~Branch()
{
	delete[] _children;
	delete[] _keys;
}

Branch::Branch(BTree* tree, INode* left, INode* right, void* key)
{
	_tree = tree;
	_children = new INode*[_tree->_branchCapacity];
	_keys = new char[(_tree->_branchCapacity - 1) * _tree->_keyType.Size];
	_children[0] = left;
	_children[1] = right;
	memcpy(&_keys[0], key, _tree->_keyType.Size);
	Count = 1;
}

InsertResult Branch::Insert(void* key, void* value, Leaf** leaf)
{
	int index = IndexOf(key);
	InsertResult result = _children[index]->Insert(key, value, leaf);

	if (result.split != NULL)
	{
		Split* temp = result.split;
		result.split = InsertChild(index, result.split->key, result.split->right);
		delete temp;
	}
	return result;
}

Split* Branch::InsertChild(int index, void* key, INode* child)
{
	if (Count != _tree->_branchCapacity - 1)
	{
		InternalInsertChild(index, key, child);
		return NULL;
	}
	else
	{
		int mid = (Count + 1) / 2;
		Branch* node = new Branch(_tree);
		node->Count = Count - mid;
		memcpy(&node->_keys[0], &_keys[mid * _tree->_keyType.Size], node->Count * _tree->_keyType.Size);
		memcpy(&node->_children[0], &_children[mid], (node->Count + 1) * sizeof(INode*));
		
		Count = mid - 1;

		Split* split = new Split();
		split->key = &_keys[(mid - 1) * _tree->_keyType.Size];
		split->right = node;

		if (index <= Count)
			InternalInsertChild(index, key, child);
		else
			node->InternalInsertChild(index - (Count + 1), key, child);

		return split;
	}
}

void Branch::InternalInsertChild(int index, void* key, INode* child)
{
	int size = Count - index;
	if (Count != index)
	{
		memmove(&_keys[(index + 1) *_tree->_keyType.Size], &_keys[index * _tree->_keyType.Size],  size * _tree->_keyType.Size);
		memmove(&_children[index + 2], &_children[index + 1],  size * sizeof(INode*));
	}

	memcpy(&_keys[index * _tree->_keyType.Size], key, _tree->_keyType.Size);
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
        result = _tree->_keyType.Compare(key, &_keys[localIndex * _tree->_keyType.Size]);

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

wstring Branch::ToString()
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

Leaf::Leaf(BTree* tree)
{
	_tree = tree;
	_keys = new char[tree->_leafCapacity * _tree->_keyType.Size];
	_values = new char[tree->_leafCapacity * _tree->_valueType.Size];
	Count = 0;
}

Leaf::~Leaf()
{
	delete[] _keys;
	delete[] _values;
}

InsertResult Leaf::Insert(void* key, void* value, Leaf** leaf)
{
	int index = IndexOf(key);
	InsertResult result;
	if (Count != _tree->_leafCapacity)
	{
		result.found = InternalInsert(index, key, value, leaf);
		result.split = NULL;
	}
	else
	{
		Leaf* node = new Leaf(_tree);
		if (index != Count)
		{
			node->Count = (_tree->_leafCapacity + 1) / 2;
			Count = Count - node->Count;

			memcpy(&node->_keys[0], &_keys[node->Count * _tree->_keyType.Size],  node->Count * _tree->_keyType.Size);
			memcpy(&node->_values[0], &_values[node->Count * _tree->_valueType.Size], node->Count * _tree->_valueType.Size);
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

void* Leaf::InternalInsert(int index, void* key, void* value, Leaf** leaf)
{
	*leaf = this;
	if (index >= Count || _tree->_keyType.Compare(&_keys[index * _tree->_keyType.Size], key) != 0)
	{
		if (Count != index)
		{
			memmove(&_keys[(index + 1) * _tree->_keyType.Size], &_keys[index], (Count - index) * _tree->_keyType.Size);
			memmove(&_values[(index + 1) * _tree->_valueType.Size], &_values[index], (Count - index) * _tree->_valueType.Size);
		}

		memcpy(&_keys[index * _tree->_keyType.Size], key, _tree->_keyType.Size);
		memcpy(&_values[index * _tree->_valueType.Size], value, _tree->_valueType.Size);
		Count++;
		return NULL;
	}
	else
		return &_values[index * _tree->_keyType.Size];
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
        result = _tree->_keyType.Compare(key, &_keys[localIndex * _tree->_keyType.Size]);

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

wstring Leaf::ToString()
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

		result << _tree->_keyType.ToString(&_keys[i * _tree->_keyType.Size]);
		result << ":";
		result << _tree->_valueType.ToString(&_values[i * _tree->_valueType.Size]);
	}
	result << "}";

	return result.str();
}

void* Leaf::GetKey(function<bool(void*)> predicate)
{
	for (int i = 0; i < Count; i++)
	{
		if (predicate(&_values[i * _tree->_valueType.Size]))
			return &_keys[i * _tree->_keyType.Size];
	}

	return NULL;
}
