#pragma once

#include <iostream>
#include <string>
#include <functional>
#include "optional.h"
#include "BTreeObserver.h"
#include "Schema/type.h"

using namespace std;

const int DefaultLeafCapacity = 128;
const int DefaultBranchCapacity = 128;

class Split;
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

class BTree
{
	public:
		BTree(type keyType, type valueType, IObserver* observer = NULL);
		~BTree();

		void* Insert(void* key, void* value, Leaf** leaf);
		wstring ToString();
		void setCapacity(int branchCapacity, int leafCapacity);
		int getBranchCapacity();
		int getLeafCapacity();

	private:
		INode* _root;
		void DoValuesMoved(Leaf*);

		int _branchCapacity;
		int _leafCapacity;
		type _keyType;
		type _valueType;
		IObserver* _observer;

	friend class Leaf;
	friend class Branch;
};

class Leaf: public INode
{
	public:
		Leaf(BTree* tree);
		~Leaf();

		InsertResult Insert(void* key, void* value, Leaf** leaf);	
		void* GetKey(function<bool(void*)>);
		wstring ToString();

	private:
		int _count;
		BTree* _tree;
		char* _keys;
		char* _values;

		int IndexOf(void* key);
		void* InternalInsert(int index, void* key, void* value, Leaf** leaf);
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

