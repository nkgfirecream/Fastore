#include "BTree.h"
#include <sstream>

using namespace std;

//Tree

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

int Branch::IndexOf(void* key)
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
	int index = IndexOf(key);
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

int Leaf::IndexOf(void* key)
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


void BTree::iterator::SeekToKey(void* value)
{
	_currentNode = _tree->_root;

	Branch* _currentBranch;
	Leaf* _currentLeaf;

	_currentIndex = _currentNode->IndexOf(value);

	_currentLeaf = dynamic_cast<Leaf*>(_currentNode);

	while(_currentLeaf == NULL)
	{
		_currentBranch = dynamic_cast<Branch*>(_currentNode);

		_path.push_back(PathNode(_currentNode, _currentIndex));

		_currentNode = _currentBranch->_children[_currentIndex];
		_currentIndex = _currentNode->IndexOf(value);
		_currentLeaf = dynamic_cast<Leaf*>(_currentNode);
	}

	_currentValue = &_currentLeaf->_values[_currentIndex * _tree->_valueType.Size];
}

void BTree::iterator::SeekToBegin()
{
	_currentNode = _tree->_root;

	Branch* _currentBranch;
	Leaf* _currentLeaf;

	_currentLeaf = dynamic_cast<Leaf*>(_currentNode);

	while(_currentLeaf == NULL)
	{
		_currentBranch = dynamic_cast<Branch*>(_currentNode);

		_path.push_back(PathNode(_currentNode, 0));			

		_currentNode = _currentBranch->_children[_currentIndex];
		_currentLeaf = dynamic_cast<Leaf*>(_currentNode);
	}

	_currentValue = &_currentLeaf->_values[_currentIndex * _tree->_valueType.Size];
}

void BTree::iterator::SeekToEnd()
{
	_currentNode = _tree->_root;

	Branch* _currentBranch;
	Leaf* _currentLeaf;
			
	_currentLeaf = dynamic_cast<Leaf*>(_currentNode);

	while(_currentLeaf == NULL)
	{				
		_currentBranch = dynamic_cast<Branch*>(_currentNode);
		_currentIndex = _currentBranch->_count;

		_path.push_back(PathNode(_currentNode, _currentIndex));

		_currentNode = _currentBranch->_children[_currentIndex];
		_currentLeaf = dynamic_cast<Leaf*>(_currentNode);
	}

	_currentIndex = _currentLeaf->_count - 1;
	_currentValue = &_currentLeaf->_values[_currentIndex * _tree->_valueType.Size];
}

void BTree::iterator::MoveNext()
{
	Leaf* _currentLeaf = dynamic_cast<Leaf*>(_currentNode);

	if(_currentIndex < _currentNode->_count - 1)
	{
		_currentIndex++;
	}
	else
	{
		//pop up until we are no longer at count
		bool pop = true;
		while(pop)
		{
			PathNode pathNode = _path[_path.size() - 1];
			_path.pop_back();
			_currentIndex = pathNode.second;
			_currentNode = pathNode.first;

			if(_currentIndex < _currentNode->_count)
				pop = false;
		}
		//increment by one
		_currentIndex++;

		//go down to the left.
		Branch* _currentBranch;

		_currentLeaf = dynamic_cast<Leaf*>(_currentNode);

		while(_currentLeaf == NULL)
		{	
			_path.push_back(PathNode(_currentNode, _currentIndex));

			_currentBranch = dynamic_cast<Branch*>(_currentNode);	

			_currentNode = _currentBranch->_children[_currentIndex];
			_currentIndex = 0;		
			_currentLeaf = dynamic_cast<Leaf*>(_currentNode);
		}		
	}

	_currentValue = &_currentLeaf->_values[_currentIndex * _tree->_valueType.Size];
}

void BTree::iterator::MovePrevious()
{
	Leaf* _currentLeaf = dynamic_cast<Leaf*>(_currentNode);

	if(_currentIndex > 0)
	{
		_currentIndex--;
	}
	else
	{
		//pop up until we are no longer at 0
		bool pop = true;
		while(pop)
		{
			PathNode pathNode = _path[_path.size() - 1];
			_path.pop_back();
			_currentIndex = pathNode.second;
			_currentNode = pathNode.first;

			if(_currentIndex > 0)
				pop = false;
		}
		//decrement by one
		_currentIndex--;

		//go down to the right.
		Branch* _currentBranch;
		Leaf* _currentLeaf;

		_currentLeaf = dynamic_cast<Leaf*>(_currentNode);

		while(_currentLeaf == NULL)
		{	
			_path.push_back(PathNode(_currentNode, _currentIndex));
			_currentBranch = dynamic_cast<Branch*>(_currentNode);
			_currentNode = _currentBranch->_children[_currentIndex];
					
			_currentIndex = _currentNode->_count - 1;
					
			_currentLeaf = dynamic_cast<Leaf*>(_currentNode);
		}		
	}

	_currentValue = &_currentLeaf->_values[_currentIndex * _tree->_valueType.Size];
}	


BTree::iterator::iterator(BTree* tree, void* key)
{
	_tree = tree;
	_currentIndex = 0;
	SeekToKey(key);
}

BTree::iterator::iterator(BTree* tree, bool begin = true)
{
	_tree = tree;
	_currentIndex = 0;

	if(begin)
		SeekToBegin();
	else
		SeekToEnd();
}

BTree::iterator::iterator(const iterator& iter) :
	_tree(iter._tree), _currentValue(iter._currentValue), _currentIndex(iter._currentIndex), _path(iter._path), _currentNode(iter._currentNode) {}

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
	MovePrevious();
	return *this;
}

BTree::iterator BTree::iterator::operator--(int)
{
	BTree::iterator tmp(*this); 
	operator--(); 
	return tmp;
}

bool BTree::iterator::operator==(const BTree::iterator& rhs) {return _currentValue==rhs._currentValue;}
bool BTree::iterator::operator!=(const BTree::iterator& rhs) {return _currentValue!=rhs._currentValue;}
void* BTree::iterator::operator*() { return _currentValue;}

