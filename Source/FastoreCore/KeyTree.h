#pragma once
#include <EASTL\string.h>
#include <functional>
#include <iterator>
#include "optional.h"
#include "Schema\type.h"

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
		virtual std::wstring ToString() = 0;
};

struct KeySplit
{
	void* key;
	IKeyNode* right;
};

//TODO: Tree iterator for all keys in KeyTree
class KeyTree
{
	public:
		KeyTree(type keyType);
		~KeyTree();

		bool Insert(void* key, KeyLeaf** leaf);

		 std::wstring ToString();

		void setCapacity(int BranchCapacity, int LeafCapacity);
		int getBranchCapacity();
		int getLeafCapacity();

	private:
		IKeyNode* _root;

		int _BranchCapacity;
		int _LeafCapacity;

		type _keyType;

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
		std::wstring ToString();
};

class KeyBranch : public IKeyNode
{
	public:
		KeyBranch(KeyTree* tree);
		KeyBranch(KeyTree* tree, IKeyNode* left, IKeyNode* right, void* key);
		~KeyBranch();

		KeyInsertResult Insert(void* key, KeyLeaf** leaf);
		std::wstring ToString();		

	private:
		int _count;
		KeyTree* _tree;
		char* _keys;
		IKeyNode** _children;

		int IndexOf(void* key);
		KeySplit* InsertChild(int index, void* key, IKeyNode* child);
		void InternalInsertChild(int index, void* key, IKeyNode* child);
};

