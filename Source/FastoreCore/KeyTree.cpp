#include "KeyTree.h"
#include "Schema\type.h"
#include "Schema\typedefs.h"
#include <sstream>

using namespace std;

//Tree

KeyTree::KeyTree(Type keyType) : 
	_keyType(keyType),
	_BranchCapacity(DefaultKeyBranchCapacity), 
	_LeafCapacity(DefaultKeyLeafCapacity)
{
	_root = new KeyLeaf(this);
}

KeyTree::~KeyTree()
{
	delete _root;
}

bool KeyTree::Insert(void* key, KeyLeaf** KeyLeaf)
{
	KeyInsertResult result = _root->Insert(key, KeyLeaf);
	if (result.split != NULL)
	{
		_root = new KeyBranch(this, _root, result.split->right, result.split->key);
		delete result.split;
	}

	return result.found;
}

// Set the Branch and Leaf capacities; only affects nodes that are created after this is set
void KeyTree::setCapacity(int KeyBranchCapacity, int KeyLeafCapacity)
{
	_BranchCapacity = KeyBranchCapacity;
	_LeafCapacity = KeyLeafCapacity;
}

int KeyTree::getBranchCapacity()
{
	return _BranchCapacity;
}

int KeyTree::getLeafCapacity()
{
	return _LeafCapacity;
}

fstring KeyTree::ToString()
{
	return _root->ToString();
}

//KeyBranch

KeyBranch::KeyBranch(KeyTree* tree)	: _tree(tree), _count(0)
{
	_children = new IKeyNode*[_tree->_BranchCapacity];
	try
	{
		_keys = new char[(_tree->_BranchCapacity - 1) * _tree->_keyType.Size];
	}
	catch (...)
	{
		delete[] _children;
		throw;
	}
}

KeyBranch::~KeyBranch()
{
	delete[] _children;
	delete[] _keys;
}

KeyBranch::KeyBranch(KeyTree* tree, IKeyNode* left, IKeyNode* right, void* key) : _tree(tree), _count(1)
{
	_children = new IKeyNode*[_tree->_BranchCapacity];
	try
	{
		_children[0] = left;
		_children[1] = right;
		_keys = new char[(_tree->_BranchCapacity - 1) * _tree->_keyType.Size];
		try
		{
			memcpy(&_keys[0], key, _tree->_keyType.Size);
		}
		catch (...)
		{
			delete[] _keys;
			throw;
		}
	}
	catch (...)
	{
		delete[] _children;
		throw;
	}
}

KeyInsertResult KeyBranch::Insert(void* key, KeyLeaf** leaf)
{
	int index = IndexOf(key);
	KeyInsertResult result = _children[index]->Insert(key, leaf);

	if (result.split != NULL)
	{
		KeySplit* temp = result.split;
		result.split = InsertChild(index, result.split->key, result.split->right);
		delete temp;
	}
	return result;
}

KeySplit* KeyBranch::InsertChild(int index, void* key, IKeyNode* child)
{
	if (_count != _tree->_BranchCapacity - 1)
	{
		InternalInsertChild(index, key, child);
		return NULL;
	}
	else
	{
		int mid = (_count + 1) / 2;
		KeyBranch* node = new KeyBranch(_tree);
		node->_count = _count - mid;
		memcpy(&node->_keys[0], &_keys[mid * _tree->_keyType.Size], node->_count * _tree->_keyType.Size);
		memcpy(&node->_children[0], &_children[mid], (node->_count + 1) * sizeof(IKeyNode*));
		
		_count = mid - 1;

		KeySplit* split = new KeySplit();
		split->key = &_keys[(mid - 1) * _tree->_keyType.Size];
		split->right = node;

		if (index <= _count)
			InternalInsertChild(index, key, child);
		else
			node->InternalInsertChild(index - (_count + 1), key, child);

		return split;
	}
}

void KeyBranch::InternalInsertChild(int index, void* key, IKeyNode* child)
{
	int size = _count - index;
	if (_count != index)
	{
		memmove(&_keys[(index + 1) *_tree->_keyType.Size], &_keys[index * _tree->_keyType.Size],  size * _tree->_keyType.Size);
		memmove(&_children[index + 2], &_children[index + 1],  size * sizeof(IKeyNode*));
	}

	memcpy(&_keys[index * _tree->_keyType.Size], key, _tree->_keyType.Size);
	_children[index + 1] = child;
	_count++;
}

int KeyBranch::IndexOf(void* key)
{	
    int lo = 0;
	int hi = _count - 1;
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

fstring KeyBranch::ToString()
{
	wstringstream result;

	result << "\n[";
	bool first = true;
	for (int i = 0; i <= _count; i++)
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

//KeyLeaf

KeyLeaf::KeyLeaf(KeyTree* tree) : _tree(tree), _count(0)
{
	_keys = new char[tree->_LeafCapacity * _tree->_keyType.Size];
}

KeyLeaf::~KeyLeaf()
{
	delete[] _keys;
}

KeyInsertResult KeyLeaf::Insert(void* key, KeyLeaf** leaf)
{
	int index = IndexOf(key);
	KeyInsertResult result;
	if (_count != _tree->_LeafCapacity)
	{
		result.found = InternalInsert(index, key, leaf);
		result.split = NULL;
	}
	else
	{
		KeyLeaf* node = new KeyLeaf(_tree);
		if (index != _count)
		{
			node->_count = (_tree->_LeafCapacity + 1) / 2;
			_count = _count - node->_count;

			memcpy(&node->_keys[0], &_keys[node->_count * _tree->_keyType.Size],  node->_count * _tree->_keyType.Size);
		}

		if (index < _count)
			result.found = InternalInsert(index, key,  leaf);
		else
			result.found = node->InternalInsert(index - _count, key,  leaf);

		KeySplit* split = new KeySplit();
		split->key = &node->_keys[0];
		split->right = node;

		result.split = split;

		return result;
	}

	return result;
}

bool KeyLeaf::InternalInsert(int index, void* key, KeyLeaf** leaf)
{
	*leaf = this;
	if (index >= _count || _tree->_keyType.Compare(&_keys[index * _tree->_keyType.Size], key) != 0)
	{
		if (_count != index)
		{
			memmove(&_keys[(index + 1) * _tree->_keyType.Size], &_keys[index  * _tree->_keyType.Size], (_count - index) * _tree->_keyType.Size);
		}

		memcpy(&_keys[index * _tree->_keyType.Size], key, _tree->_keyType.Size);

		_count++;	

		return false;
	}
	else
		return true;
}

int KeyLeaf::IndexOf(void* key)
{
	int lo = 0;
	int hi = _count - 1;
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

fstring KeyLeaf::ToString()
{
	wstringstream result;
	result << "\n{";
	bool first = true;
	for(int i = 0; i < _count; i++)
	{
		if(!first)
			result << ",";
		else
			first = false;

		result << _tree->_keyType.ToString(&_keys[i * _tree->_keyType.Size]);
	}
	result << "}";

	return result.str();
}