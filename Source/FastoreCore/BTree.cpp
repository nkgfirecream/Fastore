#include "BTree.h"
#include "Schema\scalar.h"
#include <sstream>

using namespace std;

/*
	BTree

	Assumtion: nodes will never be empty, except for a leaf when root.
*/

BTree::BTree(ScalarType keyType, ScalarType valueType) : 
	_keyType(keyType), _valueType(valueType), 
	_branchCapacity(DefaultBranchCapacity), 
	_leafCapacity(DefaultLeafCapacity)
{
	_root = new Leaf(this);
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

void BTree::DoValuesMoved(Leaf* newLeaf)
{
	if (_valuesMovedCallback != NULL)
	{
		Leaf::iterator end = newLeaf->valueEnd();
		for (Leaf::iterator i = newLeaf->valueBegin(); i != end; i++)
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
		_root = new Leaf(this);
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
		_root = new Branch(this, _root, result->right, result->key);
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

//Branch

Branch::Branch(BTree* tree)	: Node(tree)
{
	_children = new Node*[_tree->_branchCapacity];
	try
	{
		_keys = new char[(_tree->_branchCapacity - 1) * _tree->_keyType.Size];
	}
	catch (...)
	{
		delete[] _children;
		throw;
	}
}

Branch::~Branch()
{
	delete[] _children;
	delete[] _keys;
}

Branch::Branch(BTree* tree, Node* left, Node* right, void* key) : Node(tree, 1)
{
	_children = new Node*[_tree->_branchCapacity];
	try
	{
		_children[0] = left;
		_children[1] = right;
		_keys = new char[(_tree->_branchCapacity - 1) * _tree->_keyType.Size];
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

Split* Branch::Insert(int index, void* key, Node* child)
{
	if (_count != _tree->_branchCapacity - 1)
	{
		InternalInsertChild(index, key, child);
		return NULL;
	}
	else
	{
		int mid = (_count + 1) / 2;
		Branch* node = new Branch(_tree);
		node->_count = _count - mid;
		memcpy(&node->_keys[0], &_keys[mid * _tree->_keyType.Size], node->_count * _tree->_keyType.Size);
		memcpy(&node->_children[0], &_children[mid], (node->_count + 1) * sizeof(Node*));
		
		_count = mid - 1;

		Split* split = new Split();
		split->key = &_keys[(mid - 1) * _tree->_keyType.Size];
		split->right = node;

		if (index <= _count)
			InternalInsertChild(index, key, child);
		else
			node->InternalInsertChild(index - (_count + 1), key, child);

		return split;
	}
}

void Branch::InternalInsertChild(int index, void* key, Node* child)
{
	int size = _count - index;
	if (_count != index)
	{
		memmove(&_keys[(index + 1) *_tree->_keyType.Size], &_keys[index * _tree->_keyType.Size],  size * _tree->_keyType.Size);
		memmove(&_children[index + 2], &_children[index + 1],  size * sizeof(Node*));
	}

	memcpy(&_keys[index * _tree->_keyType.Size], key, _tree->_keyType.Size);
	_children[index + 1] = child;
	_count++;
}

int Branch::IndexOf(void* key)
{	
	auto result = _tree->_keyType.IndexOf(_keys, _count, key);
	return result >= 0 ? result + 1 : ~result;
}

void* Branch::GetValue(void* key, Leaf** leaf)
{
	int index = IndexOf(key);
	return _children[index]->GetValue(key, leaf);
}

fs::wstring Branch::ToString()
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

void Branch::SeekToBegin(BTree::Path& path)
{
	path.Branches.push_back(BTree::PathNode(this, 0));
	_children[0]->SeekToBegin(path);
}

void Branch::SeekToEnd(BTree::Path& path)
{
	path.Branches.push_back(BTree::PathNode(this, _count));
	_children[_count]->SeekToEnd(path);
}

bool Branch::MoveNext(BTree::Path& path)
{
	BTree::PathNode& node = path.Branches.back();
	if (node.Index < _count)
	{
		++node.Index;
		node.Node->_children[node.Index]->SeekToBegin(path);
		return true;
	}
	return false;
}

bool Branch::MovePrior(BTree::Path& path)
{
	BTree::PathNode& node = path.Branches.back();
	if (node.Index > 0)
	{
		--node.Index;
		node.Node->_children[node.Index]->SeekToEnd(path);
		return true;
	}
	return false;
}

void Branch::GetPath(void* key, BTree::Path& path)
{
	auto index = IndexOf(key);
	path.Branches.push_back(BTree::PathNode(this, index));
	_children[index]->GetPath(key, path);
}

bool Branch::Delete(int index)
{
	delete _children[index];
	int size = _count - index;
	//Assumption -- Count > 0 (otherwise the key would not have been found)
	memmove(&_keys[(index) * _tree->_keyType.Size], &_keys[(index + 1) * _tree->_keyType.Size], size * _tree->_keyType.Size);
	memmove(&_children[index], &_children[index + 1], size * sizeof(Node*));

	_count--;

	return _count < 0;
}

//Leaf
Leaf::Leaf(BTree* tree) : Node(tree)
{
	_keys = new char[tree->_leafCapacity * _tree->_keyType.Size];
	try
	{
		_values = new char[tree->_leafCapacity * _tree->_valueType.Size];
	}
	catch (...)
	{
		delete[] _keys;
		throw;
	}
}

Leaf::~Leaf()
{
	delete[] _keys;
	delete[] _values;
}

bool Leaf::Delete(int index)
{
	InternalDelete(index);
	return _count == 0;
}

Split* Leaf::Insert(int index, void* key, void* value)
{
	if (_count != _tree->_leafCapacity)
	{
		InternalInsertIndex(index, key, value);
		return NULL;
	}
	else
	{
		Leaf* node = new Leaf(_tree);
		if (index != _count)
		{
			node->_count = (_tree->_leafCapacity + 1) / 2;
			_count = _count - node->_count;

			memcpy(&node->_keys[0], &_keys[node->_count * _tree->_keyType.Size],  node->_count * _tree->_keyType.Size);
			memcpy(&node->_values[0], &_values[node->_count * _tree->_valueType.Size], node->_count * _tree->_valueType.Size);
		}

		if (index < _count)
			InternalInsertIndex(index, key, value);
		else
			node->InternalInsertIndex(index - _count, key, value);

		_tree->DoValuesMoved(node);

		Split* split = new Split();
		split->key = &node->_keys[0];
		split->right = node;

		return split;
	}
}

void Leaf::InternalInsertIndex(int index, void* key, void* value)
{
	if (_count != index)
	{
		memmove(&_keys[(index + 1) * _tree->_keyType.Size], &_keys[index  * _tree->_keyType.Size], (_count - index) * _tree->_keyType.Size);
		memmove(&_values[(index + 1) * _tree->_valueType.Size], &_values[index * _tree->_valueType.Size], (_count - index) * _tree->_valueType.Size);
	}

	memcpy(&_keys[index * _tree->_keyType.Size], key, _tree->_keyType.Size);
	memcpy(&_values[index * _tree->_valueType.Size], value, _tree->_valueType.Size);

	_count++;
}

void Leaf::InternalDelete(int index)
{
	//Who should be responsible for cleanup?
	//Probably the columnbuffer for the values...
	//Assumption -- Count > 0 (otherwise the key would not have been found)
	memmove(&_keys[(index) * _tree->_keyType.Size], &_keys[index + 1 * _tree->_keyType.Size], (_count - index) * _tree->_keyType.Size);
	memmove(&_values[(index) * _tree->_valueType.Size], &_values[index + 1 * _tree->_valueType.Size], (_count - index) * _tree->_valueType.Size);

	_count--;
}

int Leaf::IndexOf(void* key, bool& match)
{
	auto result = _tree->_keyType.IndexOf(_keys, _count, key);
	match = result >= 0;
	return match ? result : ~result;
}

void* Leaf::GetValue(void* key, Leaf** leaf)
{
	bool match;
	int index = IndexOf(key, match);
	if (match)
	{
		*leaf = this;
		return operator[](index).second;
	}
	else
	{
		return NULL;
	}
}

fs::wstring Leaf::ToString()
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
		result << ":";
		result << _tree->_valueType.ToString(&_values[i * _tree->_valueType.Size]);
	}
	result << "}";

	return result.str();
}

void* Leaf::GetKey(function<bool(void*)> predicate)
{
	for (int i = 0; i < _count; i++)
	{
		if (predicate(operator[](i).second))
			return operator[](i).first;
	}

	return NULL;
}

void Leaf::GetPath(void* key, BTree::Path& path)
{
	path.Leaf = this;
	path.LeafIndex = IndexOf(key, path.Match);
}

void Leaf::SeekToBegin(BTree::Path& path)
{
	path.Leaf = this;
	path.LeafIndex = 0;
}

void Leaf::SeekToEnd(BTree::Path& path)
{
	path.Leaf = this;
	path.LeafIndex = _count -1;
}

bool Leaf::MoveNext(BTree::Path& path)
{
	++path.LeafIndex;
	if (path.LeafIndex < _count)
		return true;
	else
	{
		int depth =  - 1;
		// walk up until we are no longer at the end
		while (path.Branches.size() > 0)
		{
			BTree::PathNode& node = path.Branches.back();

			if (node.Node->MoveNext(path))
				return true;
			else
				path.Branches.pop_back();
		}
		return false;
	}
}

bool Leaf::MovePrior(BTree::Path& path)
{
	--path.LeafIndex;
	if (path.LeafIndex >= 0)
		return true;
	else
	{
		int depth =  - 1;
		// walk up until we are no longer at the beginning
		while (path.Branches.size() > 0)
		{
			BTree::PathNode& node = path.Branches.back();

			if (node.Node->MovePrior(path))
				return true;
			else
				path.Branches.pop_back();
		}
		return false;
	}
}

bool Leaf::EndOfTree(BTree::Path& path)
{
	return path.LeafIndex == path.Leaf->_count && path.Branches.size() == 0;
}

bool Leaf::BeginOfTree(BTree::Path& path)
{
	return path.LeafIndex < 0 && path.Branches.size() == 0;
}

eastl::pair<void*,void*> Leaf::operator[](int index)
{
	return eastl::pair<void*,void*>(_keys + (_tree->_keyType.Size * index), _values + (_tree->_valueType.Size * index));
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

bool BTree::iterator::operator==(const BTree::iterator& rhs) {return (_path.Leaf == rhs._path.Leaf) && (_path.LeafIndex == rhs._path.LeafIndex);}
bool BTree::iterator::operator!=(const BTree::iterator& rhs) {return (_path.Leaf != rhs._path.Leaf) || (_path.LeafIndex != rhs._path.LeafIndex);}
eastl::pair<void*,void*> BTree::iterator::operator*() { return (*(_path.Leaf))[_path.LeafIndex];}

