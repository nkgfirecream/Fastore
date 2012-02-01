#pragma once
#include <EASTL\string.h>
#include <functional>
#include <iterator>
#include "optional.h"
#include "BTreeObserver.h"
#include "Schema/type.h"

using namespace std;

const int DefaultLeafCapacity = 128;
const int DefaultBranchCapacity = 128;

struct Split;
class Leaf;

struct InsertResult
{
	void* found;
	Split* split;
};

class INode
{
	public:
		virtual ~INode() {}
		virtual InsertResult Insert(void* key, void* value, Leaf** leaf) = 0;
		virtual wstring ToString() = 0;
};

struct Split
{
	void* key;
	INode* right;
};

typedef void (*valuesMovedHandler)(void* value, Leaf& newLeaf);

class BTree
{
	public:
		BTree(Type keyType, Type valueType);
		~BTree();

		void* Insert(void* key, void* value, Leaf** leaf);

		wstring ToString();

		void setCapacity(int branchCapacity, int leafCapacity);
		int getBranchCapacity();
		int getLeafCapacity();

		void setValuesMovedCallback(valuesMovedHandler callback);
		valuesMovedHandler getValuesMovedCallback();

	private:
		INode* _root;

		void DoValuesMoved(Leaf& newLeaf);
		valuesMovedHandler _valuesMovedCallback;

		int _branchCapacity;
		int _leafCapacity;

		Type _keyType;
		Type _valueType;

	friend class Leaf;
	friend class Branch;
};

class Leaf: public INode
{
		int _count;
		BTree* _tree;
		char* _keys;
		char* _values;

		int IndexOf(void* key);
		void* InternalInsert(int index, void* key, void* value, Leaf** leaf);

	public:
		Leaf(BTree* tree);
		~Leaf();

		InsertResult Insert(void* key, void* value, Leaf** leaf);	
		void* GetKey(function<bool(void*)>);
		wstring ToString();

		class iterator : public std::iterator<input_iterator_tag, void*>
		{
				char* _item;
				int _size;
				iterator(char* item, int size) : _item(item), _size(size) {}
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
};

class Branch : public INode
{
	public:
		Branch(BTree* tree);
		Branch(BTree* tree, INode* left, INode* right, void* key);
		~Branch();

		InsertResult Insert(void* key, void* value, Leaf** leaf);
		wstring ToString();		

	private:
		int _count;
		BTree* _tree;
		char* _keys;
		INode** _children;

		int IndexOf(void* key);
		Split* InsertChild(int index, void* key, INode* child);
		void InternalInsertChild(int index, void* key, INode* child);
};

