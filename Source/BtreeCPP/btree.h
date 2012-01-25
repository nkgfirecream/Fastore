#include <iostream>
#include <string>
using namespace std;

class Split;

class InsertResult
{
	public:
		void* found;
		Split* split;
};

class INode
{
	public:
		virtual ~INode() {}
		virtual InsertResult Insert(void* key, void* value) = 0;
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
		int (*Compare)(void* left, void* right);
		wstring (*ItemToString)(void* item);
	
		BTree(int fanout, int leafsize, int(*)(void*,void*), wstring(*)(void*));

		void* Insert(void* key, void* value);
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

		InsertResult INode::Insert(void* key, void* value);	
		wstring INode::ToString();

	private:
		int IndexOf(void* key);
		void* InternalInsert(int index, void* key, void* value);

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

		InsertResult INode::Insert(void* key, void* value);
		wstring INode::ToString();		

	private:
		int IndexOf(void* key);
		Split* InsertChild(int index, void* key, INode* child);
		void InternalInsertChild(int index, void* key, INode* child);
		
};




