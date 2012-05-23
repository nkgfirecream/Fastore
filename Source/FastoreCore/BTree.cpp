#include "BTree.h"
#include "Schema\scalar.h"
#include <sstream>
#include "typedefs.h"

using namespace std;

/*
	BTree

	Assumtion: nodes will never be empty, except for a leaf when root.
*/

template<> void CopyToArray<Node*>(const void* item, void* arrpointer)
{
	memcpy(arrpointer, item, sizeof(Node*));
}

ScalarType GetNodeType()
{
	ScalarType type;
	type.CopyIn = CopyToArray<Node*>;
	type.Size = sizeof(Node*);
	return type;
}


BTree::BTree(ScalarType keyType, ScalarType valueType) : 
	_keyType(keyType), _valueType(valueType)
{
	_nodeType = GetNodeType();
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

BTree::Path BTree::SeekToBegin()
{
	Path result;
	_root->SeekToBegin(result);
	return result;
}

BTree::Path BTree::SeekToEnd()
{
	Path result;
	_root->SeekToEnd(result);
	return result;
}

// BTree iterator

bool BTree::iterator::MoveNext()
{
	return _path.Leaf->MoveNext(_path);
}

bool BTree::iterator::MovePrior()
{
	return _path.Leaf->MovePrior(_path);
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

bool BTree::iterator::End()
{
	return _path.Leaf->EndOfTree(_path);
}

bool  BTree::iterator::Begin()
{
	return _path.Leaf->BeginOfTree(_path);
}

bool BTree::iterator::TestPath()
{
	return _path.LeafIndex < (*(_path.Leaf))._count;
}

bool BTree::iterator::operator==(const BTree::iterator& rhs) {return (_path.Leaf == rhs._path.Leaf) && (_path.LeafIndex == rhs._path.LeafIndex);}
bool BTree::iterator::operator!=(const BTree::iterator& rhs) {return (_path.Leaf != rhs._path.Leaf) || (_path.LeafIndex != rhs._path.LeafIndex);}
TreeEntry BTree::iterator::operator*() { return (*(_path.Leaf))[_path.LeafIndex];}



