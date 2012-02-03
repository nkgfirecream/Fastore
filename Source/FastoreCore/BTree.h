#pragma once
#include "Schema\scalar.h"
#include "Schema\typedefs.h"
#include <functional>
#include "optional.h"
#include <EASTL\vector.h>

using namespace std;
const int DefaultLeafCapacity = 128;
const int DefaultBranchCapacity = 128;

struct Split;
class Node;
class Leaf;
class BTree;
class Branch;

struct InsertResult
{
	void* found;
	Split* split;
};

struct Split
{
	void* key;
	Node* right;
};

typedef function<void(void*,Leaf&)> valuesMovedHandler;

class BTree
{
	public:
		BTree(ScalarType keyType, ScalarType valueType);
		~BTree();

		void* Insert(void* key, void* value, Leaf** leaf);
		fs::wstring ToString();
		void setCapacity(int branchCapacity, int leafCapacity);
		int getBranchCapacity();
		int getLeafCapacity();

		void setValuesMovedCallback(valuesMovedHandler callback);
		valuesMovedHandler getValuesMovedCallback();

		class iterator : public std::iterator<bidirectional_iterator_tag, void*>
		{
			typedef eastl::pair<Node*,int> PathNode;
			private:
				BTree* _tree;
				Node* _currentNode;
				int	_currentIndex;
				void* _currentValue;
				eastl::vector<PathNode> _path;

				void SeekToKey(void* value);
				void SeekToBegin();
				void SeekToEnd();
				void MoveNext();
				void MovePrevious();
			
			public:
				iterator(BTree*, bool);
				iterator(BTree*, void*);
				iterator(const iterator& iter);
				iterator& operator++();
				iterator operator++(int);
				iterator& operator--();
				iterator operator--(int);
				bool operator==(const iterator& rhs);
				bool operator!=(const iterator& rhs);
				void* operator*();
		};

		iterator begin()
		{
			return iterator(this, true);
		}

		iterator end()
		{
			return iterator(this, false);
		}

		iterator find(void* key)
		{
			return iterator(this,key);
		}


	private:
		Node* _root;
		int _branchCapacity;
		int _leafCapacity;
		ScalarType _keyType;
		ScalarType _valueType;
		
		void DoValuesMoved(Leaf& newLeaf);
		valuesMovedHandler _valuesMovedCallback;

	friend class Leaf;
	friend class Branch;
	friend class BTree::iterator;
};

class Node
{
	public:
		Node(BTree* tree, int count = 0) : _tree(tree), _count(count) {}
		virtual ~Node() {}
		virtual InsertResult Insert(void* key, void* value, Leaf** leaf) = 0;
		virtual fs::wstring ToString() = 0;
		virtual int IndexOf(void*) = 0;

	protected:	
		int _count;
		char* _keys;
		BTree* _tree;

	friend class BTree::iterator;
};


class Leaf: public Node
{	
	private:		
		char* _values;
		int IndexOf(void* key);
		void* InternalInsert(int index, void* key, void* value, Leaf** leaf);

	public:
		Leaf(BTree* tree);
		~Leaf();

		InsertResult Insert(void* key, void* value, Leaf** leaf);	
		void* GetKey(function<bool(void*)>);
		fs::wstring ToString();

		class iterator : public std::iterator<input_iterator_tag, void*>
		{
				char* _item;
				size_t _size;
				iterator(char* item, size_t size) : _item(item), _size(size) {}
			public:
				iterator(const iterator& iter) : _item(iter._item) {}
				iterator& operator++() 
				{
					_item += _size; 
					return *this;
				}
				iterator operator++(int)
				{
					iterator tmp(*this); 
					operator++(); 
					return tmp;
				}
				bool operator==(const iterator& rhs) {return _item==rhs._item;}
				bool operator!=(const iterator& rhs) {return _item!=rhs._item;}
				void* operator*() { return _item;}
			friend class Leaf;
		};

		iterator valueBegin()
		{
			return iterator(_values, _tree->_valueType.Size);
		}

		iterator valueEnd()
		{
			return iterator(_values + _tree->_valueType.Size * _count, _tree->_valueType.Size);
		}

	friend class BTree::iterator;
};

class Branch : public Node
{
	public:
		Branch(BTree* tree);
		Branch(BTree* tree, Node* left, Node* right, void* key);
		~Branch();

		InsertResult Insert(void* key, void* value, Leaf** leaf);
		fs::wstring ToString();		

	private:
		Node** _children;

		int IndexOf(void* key);
		Split* InsertChild(int index, void* key, Node* child);
		void InternalInsertChild(int index, void* key, Node* child);

	friend class BTree::iterator;
};


