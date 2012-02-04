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

void BTree::DoValuesMoved(Leaf& newLeaf)
{
	if (_valuesMovedCallback != NULL)
	{
		Leaf::iterator end = newLeaf.valueEnd();
		for (Leaf::iterator i = newLeaf.valueBegin(); i != end; i++)
			_valuesMovedCallback(*i, newLeaf);
	}
}

fs::wstring BTree::ToString()
{
	return _root->ToString();
}

BTree::Path BTree::SeekToKey(void* key, bool forward)
{
	Path result;
	_root->SeekToKey(key, result, forward);
	return result;
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

InsertResult Branch::Insert(void* key, void* value, Leaf** leaf)
{
	int index = IndexOf(key, true);
	InsertResult result = _children[index]->Insert(key, value, leaf);

	if (result.split != NULL)
	{
		Split* temp = result.split;
		result.split = InsertChild(index, result.split->key, result.split->right);
		delete temp;
	}
	return result;
}

Split* Branch::InsertChild(int index, void* key, Node* child)
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

int Branch::IndexOf(void* key, bool forward)
{	
    int lo = 0;
	int hi = _count - 1;
	int localIndex = 0;
	int result = -1;

	while (lo <= hi)
	{
		localIndex = (lo + hi) >> 1;   // EASTL says: We use '>>1' here instead of '/2' because MSVC++ for some reason generates significantly worse code for '/2'. Go figure.
        result = _tree->_keyType.Compare(key, &_keys[localIndex * _tree->_keyType.Size]);

		if (result == 0)
			return localIndex;
		else if (result < 0)
			hi = localIndex - 1;
		else
			lo = localIndex + 1;
	}

    return lo + forward;
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
	BTree::PathNode result;
	result.Index = 0;
	result.Node = this;
	path.Branches.push_back(result);
	if (result.Index < _count)
		_children[result.Index]->SeekToBegin(path);
}

void Branch::SeekToEnd(BTree::Path& path)
{
	BTree::PathNode result;
	result.Index = _count;
	result.Node = this;
	path.Branches.push_back(result);
	if (result.Index <= _count)
		_children[result.Index]->SeekToEnd(path);
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

void Branch::SeekToKey(void* key, BTree::Path& path, bool forward)
{
	BTree::PathNode result;
	result.Index = IndexOf(key, forward);
	result.Node = this;
	path.Branches.push_back(result);
	if (result.Index < _count)
		_children[result.Index]->SeekToKey(key, path, forward);
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

InsertResult Leaf::Insert(void* key, void* value, Leaf** leaf)
{
	int index = IndexOf(key, true);
	InsertResult result;
	if (_count != _tree->_leafCapacity)
	{
		result.found = InternalInsert(index, key, value, leaf);
		result.split = NULL;
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
			result.found = InternalInsert(index, key, value, leaf);
		else
			result.found = node->InternalInsert(index - _count, key, value, leaf);

		_tree->DoValuesMoved(*node);

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
	if (index >= _count || _tree->_keyType.Compare(&_keys[index * _tree->_keyType.Size], key) != 0)
	{
		if (_count != index)
		{
			memmove(&_keys[(index + 1) * _tree->_keyType.Size], &_keys[index  * _tree->_keyType.Size], (_count - index) * _tree->_keyType.Size);
			memmove(&_values[(index + 1) * _tree->_valueType.Size], &_values[index * _tree->_valueType.Size], (_count - index) * _tree->_valueType.Size);
		}

		memcpy(&_keys[index * _tree->_keyType.Size], key, _tree->_keyType.Size);
		memcpy(&_values[index * _tree->_valueType.Size], value, _tree->_valueType.Size);

		_count++;	

		return NULL;
	}
	else
		return &_values[index * _tree->_keyType.Size];
}

int Leaf::IndexOf(void* key, bool forward)
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
			return localIndex;
		else if (result < 0)
			hi = localIndex - 1;
		else
			lo = localIndex + 1;
	}

    return lo + forward;
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
		if (predicate(&_values[i * _tree->_valueType.Size]))
			return &_keys[i * _tree->_keyType.Size];
	}

	return NULL;
}

void Leaf::SeekToKey(void* key, BTree::Path& path, bool forward)
{
	path.Leaf = this;
	path.LeafIndex = IndexOf(key, forward);
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
	if (path.LeafIndex < _count - 1)
	{
		++path.LeafIndex;
		return true;
	}
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

//TODO: Complete this Code.
bool Leaf::MovePrior(BTree::Path& path)
{
	return true;
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

bool BTree::iterator::operator==(const BTree::iterator& rhs) {return (_path.Leaf == rhs._path.Leaf) && (_path.LeafIndex == rhs._path.LeafIndex);}
bool BTree::iterator::operator!=(const BTree::iterator& rhs) {return (_path.Leaf != rhs._path.Leaf) || (_path.LeafIndex != rhs._path.LeafIndex);}
void* BTree::iterator::operator*() { return &_path.Leaf[_path.LeafIndex];}

