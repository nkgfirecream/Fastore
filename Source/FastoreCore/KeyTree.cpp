#include "KeyTree.h"
#include "Schema\scalar.h"
#include <sstream>
#include "typedefs.h"

using namespace std;

/*
	KeyTree

	Assumtion: nodes will never be empty, except for a leaf when root.
*/

template<> void CopyToArray<KeyNode*>(const void* item, void* arrpointer)
{
	memcpy(arrpointer, item, sizeof(KeyNode*));
}

ScalarType GetKeyNodeType()
{
	ScalarType type;
	type.CopyIn = CopyToArray<KeyNode*>;
	type.Size = sizeof(KeyNode*);
	return type;
}

KeyTree::KeyTree(ScalarType keyType) : 
	_keyType(keyType)
{
	_count = 0;
	_nodeType = GetKeyNodeType();
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

KeyTree::Path KeyTree::SeekToBegin()
{
	Path result;
	_root->SeekToBegin(result);
	return result;
}

KeyTree::Path KeyTree::SeekToEnd()
{
	Path result;
	_root->SeekToEnd(result);
	return result;
}

// KeyTree iterator

bool KeyTree::iterator::MoveNext()
{
	return _path.Leaf->MoveNext(_path);
}

bool KeyTree::iterator::MovePrior()
{
	return _path.Leaf->MovePrior(_path);
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

bool KeyTree::iterator::End()
{
	return _path.Leaf->EndOfTree(_path);
}

bool  KeyTree::iterator::Begin()
{
	return _path.Leaf->BeginOfTree(_path);
}

bool KeyTree::iterator::TestPath()
{
	return _path.LeafIndex < (*(_path.Leaf))._count;
}

bool KeyTree::iterator::operator==(const KeyTree::iterator& rhs) {return (_path.Leaf == rhs._path.Leaf) && (_path.LeafIndex == rhs._path.LeafIndex);}
bool KeyTree::iterator::operator!=(const KeyTree::iterator& rhs) {return (_path.Leaf != rhs._path.Leaf) || (_path.LeafIndex != rhs._path.LeafIndex);}
KeyTreeEntry KeyTree::iterator::operator*() { return (*(_path.Leaf))[_path.LeafIndex];}



