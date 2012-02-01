#pragma once
#include <EASTL\string.h>
#include <functional>
#include <iterator>
#include "optional.h"
#include "Schema/type.h"

using namespace std;

const int DefaultKeyLeafCapacity = 128;
const int DefaultKeyBranchCapacity = 128;

struct KeySplit;
class KeyLeaf;

struct KeyInsertResult
{
	bool found;
	KeySplit* split;
};

class IKeyNode
{
	public:
		virtual ~IKeyNode() {}
		virtual KeyInsertResult Insert(void* key, KeyLeaf** KeyLeaf) = 0;
		virtual wstring ToString() = 0;
};

struct KeySplit
{
	void* key;
	IKeyNode* right;
};

typedef void (*valuesMovedHandler)(void* value, KeyLeaf& newKeyLeaf);

class KeyTree
{
	public:
		KeyTree(Type keyType);
		~KeyTree();

		bool Insert(void* key, KeyLeaf** leaf);

		wstring ToString();

		void setCapacity(int BranchCapacity, int LeafCapacity);
		int getBranchCapacity();
		int getLeafCapacity();

		void setValuesMovedCallback(valuesMovedHandler callback);
		valuesMovedHandler getValuesMovedCallback();

	private:
		IKeyNode* _root;

		void DoValuesMoved(KeyLeaf& newKeyLeaf);
		valuesMovedHandler _valuesMovedCallback;

		int _BranchCapacity;
		int _LeafCapacity;

		Type _keyType;

	friend class KeyLeaf;
	friend class KeyBranch;
};

class KeyLeaf: public IKeyNode
{
		int _count;
		KeyTree* _tree;
		char* _keys;

		int IndexOf(void* key);
		bool InternalInsert(int index, void* key, KeyLeaf** leaf);

	public:
		KeyLeaf(KeyTree* tree);
		~KeyLeaf();

		KeyInsertResult Insert(void* key, KeyLeaf** leaf);	
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
			friend class KeyLeaf;
		};

		iterator keyBegin()
		{
			return iterator(_keys, _tree->_keyType.Size);
		}

		iterator keyEnd()
		{
			return iterator(_keys + _tree->_keyType.Size * _count, _tree->_keyType.Size);
		}
};

class KeyBranch : public IKeyNode
{
	public:
		KeyBranch(KeyTree* tree);
		KeyBranch(KeyTree* tree, IKeyNode* left, IKeyNode* right, void* key);
		~KeyBranch();

		KeyInsertResult Insert(void* key, KeyLeaf** leaf);
		wstring ToString();		

	private:
		int _count;
		KeyTree* _tree;
		char* _keys;
		IKeyNode** _children;

		int IndexOf(void* key);
		KeySplit* InsertChild(int index, void* key, IKeyNode* child);
		void InternalInsertChild(int index, void* key, IKeyNode* child);
};

