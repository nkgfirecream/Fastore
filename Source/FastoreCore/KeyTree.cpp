#include "KeyTree.h"
#include "Schema\scalar.h"
#include <sstream>
#include "typedefs.h"

using namespace std;

/*
	KeyTree

	Assumtion: nodes will never be empty, except for a leaf when root.
*/

KeyTree::KeyTree(ScalarType keyType) : 
	_keyType(keyType)
{
	_count = 0;
	_nodeType = KeyNodeType();
	_root = new KeyNode(this);
}

KeyTree::~KeyTree()
{
	delete _root;
}

KeyTree::Path KeyTree::GetPath(void* key)
{
	Path result;
	_root->GetPath(key, result);
	return result;
}

fs::wstring KeyTree::ToString()
{
	return _root->ToString();
}

void KeyTree::Delete(Path& path)
{
	bool result = path.Leaf->Delete(path.LeafIndex);

	if (result)
		_count--;

	while (result && path.Branches.size() > 0)
	{
		auto pathNode = path.Branches.back();
		path.Branches.pop_back();
		result = pathNode.Node->Delete(pathNode.Index);
	}

	//We got to the root and removed everything
	if (result)
	{
		_root = new KeyNode(this);
		_count = 0;
	}
}

void KeyTree::Insert(Path& path, void* key)
{
	KeySplit* result = path.Leaf->Insert(path.LeafIndex, key, (void*)NULL);
	//TODO: Add insert result to get this right. result is not null if there is a split, but we need to know if something was added.
	if (result != NULL)
		_count++;

	while (result != NULL && path.Branches.size() > 0)
	{
		auto pathNode = path.Branches.back();
		path.Branches.pop_back();
		void* key = result->key;
		KeyNode* node = result->right;
		delete result;

		result = pathNode.Node->Insert(pathNode.Index, key, node);
	}

	if (result != NULL)
	{
		_root = new KeyNode(this, 1, 1, _root, result->right, result->key);
		delete result;
	}
}

KeyTree::Path KeyTree::SeekToFirst()
{
	Path result;
	_root->SeekToFirst(result);
	return result;
}

KeyTree::Path KeyTree::SeekToLast()
{
	Path result;
	_root->SeekToLast(result);
	return result;
}

// KeyTree iterator
KeyTree::iterator::iterator(const KeyTree::Path& path, bool eofOnEmpty = false)
	: _path(path), _eof(false)
{
	//Validate path: Iterators can only ever point at the end of the BTree (one index past the end of the last leaf)
	//or at an item in the tree. All other paths are invalid. (they may be valid for insertion, such as adding an item
	//to the end of the Leaf, but insertion and iteration are different concepts).
	//Don't validate on empty tree, because we'll throw an exception.
	if ((_path.LeafIndex >= _path.Leaf->_count) && !(_path.Branches.size() == 0 && _path.Leaf->_count == 0))
		MoveNext();

	if (eofOnEmpty && (_path.Branches.size() == 0 && _path.Leaf->_count == 0))
		_eof = true;
}

bool KeyTree::iterator::MoveNext()
{
	if(_eof)
		throw "Iteration past end of tree";

	//If we are at the maximum for this leaf, search path for node with greater values
	//If we find one, rebuild path
	//If not, set pointer to one past end of tree and set eof flag.
	if (_path.LeafIndex >= _path.Leaf->_count - 1)
	{
		int nummaxed = 0;
		bool open = false;
		int size = _path.Branches.size();
		while (nummaxed < size && !open)
		{
			KeyTree::PathNode& pnode = _path.Branches.at(size - 1 - nummaxed);
			if (pnode.Index == pnode.Node->_count)
				++nummaxed;
			else
				open = true;
		}

		if (open == false)
		{
			_path.LeafIndex = _path.Leaf->_count;
			_eof = true;
		}
		else
		{
			for(int i = 0; i < nummaxed; i++)
			{
				_path.Branches.pop_back();
			}

			KeyTree::PathNode& pnode = _path.Branches.back();
			++pnode.Index;
			int size = ((KeyNode*)pnode.Node)->_valueType.Size;
			(*(KeyNode**)&(((KeyNode*)pnode.Node)->_values[pnode.Index * size]))->SeekToFirst(_path);
		}
	}
	else
		++_path.LeafIndex;
}

bool KeyTree::iterator::MovePrior()
{
	if (_path.LeafIndex == 0)
	{
		//We are at the minimum for this leaf. Check the path for any nodes with lesser values
		//If we find one, rebuild the path.
		//If not, throw exception (iteration past beginning).
		int nummin = 0;
		bool open = false;
		int size = _path.Branches.size();
		while (nummin < size && !open)
		{
			KeyTree::PathNode& pnode = _path.Branches.at(size - 1 - nummin);
			if (pnode.Index == 0)
				++nummin;
			else
				open = true;
		}

		if (open == false)
			throw "Iteration past start of tree";
		else
		{
			for(int i = 0; i < nummin; i++)
			{
				_path.Branches.pop_back();
			}

			KeyTree::PathNode& pnode = _path.Branches.back();
			--pnode.Index;
			int size = ((KeyNode*)pnode.Node)->_valueType.Size;
			(*(KeyNode**)&(((KeyNode*)pnode.Node)->_values[pnode.Index * size]))->SeekToLast(_path);
			_eof = false;
		}
	}
	else
	{
		--_path.LeafIndex;
		_eof = false;
	}
}

KeyTree::iterator& KeyTree::iterator::operator++() 
{
	MoveNext();
	return *this;
}

KeyTree::iterator KeyTree::iterator::operator++(int)
{
	KeyTree::iterator tmp(*this); 
	operator++(); 
	return tmp;
}

KeyTree::iterator& KeyTree::iterator::operator--() 
{
	MovePrior();
	return *this;
}

KeyTree::iterator KeyTree::iterator::operator--(int)
{
	KeyTree::iterator tmp(*this); 
	operator--(); 
	return tmp;
}

bool KeyTree::iterator::operator==(const KeyTree::iterator& rhs) {return (_path.Leaf == rhs._path.Leaf) && (_path.LeafIndex == rhs._path.LeafIndex) && _eof == rhs._eof;;}
bool KeyTree::iterator::operator!=(const KeyTree::iterator& rhs) {return (_path.Leaf != rhs._path.Leaf) || (_path.LeafIndex != rhs._path.LeafIndex) || _eof != rhs._eof;}
KeyTreeEntry KeyTree::iterator::operator*()
{
	if (_path.LeafIndex >= _path.Leaf->_count)
		throw "Iterator not dereferenceable";

	return (*(_path.Leaf))[_path.LeafIndex];
}

void DeallocateKeyNode(void* items, int count)
{
	// TODO: How to call a hash_set destructor?
	for (int i = 0; i < count; i++)
		((KeyNode*)items)[i].~KeyNode();
}

template<> void CopyToArray<KeyNode*>(const void* item, void* arrpointer)
{
	memcpy(arrpointer, item, sizeof(KeyNode*));
}

KeyNodeType::KeyNodeType()
{
	CopyIn = CopyToArray<KeyNode*>;
	Name = "KeyNodeType";
	Size = sizeof(KeyNode*);
	Deallocate = DeallocateKeyNode;
}

NoOpKeyNodeType::NoOpKeyNodeType()
{
	CopyIn = CopyToArray<KeyNode*>;
	Name = "KeyNodeType";
	Size = sizeof(KeyNode*);
	Deallocate = NoOpDeallocate;
}



