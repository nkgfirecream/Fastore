#include "BTree.h"
#include "Schema\scalar.h"
#include <sstream>
#include "typedefs.h"

using namespace std;

/*
	BTree

	Assumtion: nodes will never be empty, except for a leaf when root.
*/

BTree::BTree(ScalarType keyType, ScalarType valueType) : 
	_keyType(keyType), _valueType(valueType)
{
	_nodeType = NodeType();
	_root = new Node(this);
}

BTree::~BTree()
{
	delete _root;
}

BTree::Path BTree::GetPath(void* key)
{
	Path result;
	_root->GetPath(key, result);
	return result;
}

void BTree::DoValuesMoved(Node* newLeaf)
{
	if (_valuesMovedCallback != NULL)
	{
		Node::iterator end = newLeaf->valueEnd();
		for (Node::iterator i = newLeaf->valueBegin(); i != end; i++)
			_valuesMovedCallback(*i, newLeaf);
	}
}

void BTree::setValuesMovedCallback(valuesMovedHandler callback)
{
	_valuesMovedCallback = callback;
}

fs::wstring BTree::ToString()
{
	return _root->ToString();
}

void BTree::Delete(Path& path)
{
	bool result = path.Leaf->Delete(path.LeafIndex);
	while (result && path.Branches.size() > 0)
	{
		auto pathNode = path.Branches.back();
		path.Branches.pop_back();
		result = pathNode.Node->Delete(pathNode.Index);
	}

	//We got to the root and removed everything
	if (result)
		_root = new Node(this);
}

void BTree::Insert(Path& path, void* key, void* value)
{
	Split* result = path.Leaf->Insert(path.LeafIndex, key, value);
	while (result != NULL && path.Branches.size() > 0)
	{
		auto pathNode = path.Branches.back();
		path.Branches.pop_back();
		void* key = result->key;
		Node* node = result->right;
		delete result;

		result = pathNode.Node->Insert(pathNode.Index, key, node);
	}

	if (result != NULL)
	{
		_root = new Node(this, 1, 1, _root, result->right, result->key);
		delete result;
	}
}

BTree::Path BTree::SeekToFirst()
{
	Path result;
	_root->SeekToFirst(result);
	return result;
}

BTree::Path BTree::SeekToLast()
{
	Path result;
	_root->SeekToLast(result);
	return result;
}

// BTree iterator
BTree::iterator::iterator(const BTree::Path& path, bool eofOnEmpty = false)
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

void BTree::iterator::MoveNext()
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
			BTree::PathNode& pnode = _path.Branches.at(size - 1 - nummaxed);
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

			BTree::PathNode& pnode = _path.Branches.back();
			++pnode.Index;
			(*(Node**)((*(Node*)pnode.Node)[pnode.Index].value))->SeekToFirst(_path);
		}
	}
	else
		++_path.LeafIndex;
}

void BTree::iterator::MovePrior()
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
			BTree::PathNode& pnode = _path.Branches.at(size - 1 - nummin);
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

			BTree::PathNode& pnode = _path.Branches.back();
			--pnode.Index;
			(*(Node**)((*(Node*)pnode.Node)[pnode.Index].value))->SeekToLast(_path);
			_eof = false;
		}
	}
	else
	{
		--_path.LeafIndex;
		_eof = false;
	}
}

BTree::iterator& BTree::iterator::operator++() 
{
	MoveNext();
	return *this;
}

BTree::iterator BTree::iterator::operator++(int)
{
	BTree::iterator tmp(*this); 
	operator++(); 
	return tmp;
}

BTree::iterator& BTree::iterator::operator--() 
{
	MovePrior();
	return *this;
}

BTree::iterator BTree::iterator::operator--(int)
{
	BTree::iterator tmp(*this); 
	operator--(); 
	return tmp;
}

bool BTree::iterator::operator==(const BTree::iterator& rhs) {return (_path.Leaf == rhs._path.Leaf) && (_path.LeafIndex == rhs._path.LeafIndex) && _eof == rhs._eof;}
bool BTree::iterator::operator!=(const BTree::iterator& rhs) {return (_path.Leaf != rhs._path.Leaf) || (_path.LeafIndex != rhs._path.LeafIndex) || _eof != rhs._eof;}
TreeEntry BTree::iterator::operator*()
{ 
	if (_path.LeafIndex >= _path.Leaf->_count)
		throw "Iterator not dereferenceable";

	return (*(_path.Leaf))[_path.LeafIndex];
}

void DeallocateNode(void* items, int count)
{
	// TODO: How to call a hash_set destructor?
	for (int i = 0; i < count; i++)
		((Node*)items)[i].~Node();
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
	Name = "NodeType";
	Size = sizeof(Node*);
	Deallocate = NoOpDeallocate;
}




