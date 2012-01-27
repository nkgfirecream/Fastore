#pragma once

#include <iostream>
#include <string>
#include <functional>
#include "BTreeObserver.h"

using namespace std;

class Split;
class Leaf;

class InsertResult
{
	public:
		void* found;
		Split* split;
};

//Base class depends on derived class (consider refactoring)
class INode
{
	public:
		virtual ~INode() {}
		virtual InsertResult Insert(void* key, void* value, Leaf* leaf) = 0;
		virtual wstring ToString() = 0;
};

class Split
{
	public:
		void* key;
		INode* right;
};

class BTree
{
	public:
		int Fanout;
		int LeafSize;
		wstring (*ItemToString)(void* item);
		int (*Compare)(void* left, void* right);
		IObserver* Observer;
	
		BTree(int fanout, int leafsize, int(*)(void*,void*), wstring(*)(void*));

		void* Insert(void* key, void* value, Leaf* leaf);
		void DoValuesMoved(Leaf*);
		wstring ToString();

	private:
		INode* _root;
};

class Leaf: public INode
{
	public:
		int Count;
		BTree* _tree;
		void** _keys;
		void** _values;

		Leaf(BTree* tree);

		InsertResult INode::Insert(void* key, void* value, Leaf* leaf);	
		void* GetKey(function<bool(void*)>);
		wstring INode::ToString();

	private:
		int IndexOf(void* key);
		void* InternalInsert(int index, void* key, void* value, Leaf* leaf);

};

class Branch : public INode
{
	public:
		int Count;
		BTree* _tree;
		void** _keys;
		INode** _children;

		Branch(BTree* tree);
		Branch(BTree* tree, INode* left, INode* right, void* key);

		InsertResult INode::Insert(void* key, void* value, Leaf* leaf);
		wstring INode::ToString();		

	private:
		int IndexOf(void* key);
		Split* InsertChild(int index, void* key, INode* child);
		void InternalInsertChild(int index, void* key, INode* child);
		
};




