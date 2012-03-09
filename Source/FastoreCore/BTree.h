#pragma once
#include "Schema\scalar.h"
#include "typedefs.h"
#include <functional>
#include "optional.h"
#include <EASTL\vector.h>
#include <EASTL\list.h>
#include "Util\utilities.h"

using namespace std;
const int DefaultLeafCapacity = 300;
const int DefaultBranchCapacity = 300;

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

struct DeleteResult
{
	bool found;
	bool empty;
};

struct Split
{
	void* key;
	Node* right;
};

typedef function<void(void*,Leaf*)> valuesMovedHandler;

class BTree
{
	public:
		BTree(ScalarType keyType, ScalarType valueType);
		~BTree();

		fs::wstring ToString();
		void setCapacity(int branchCapacity, int leafCapacity);
		int getBranchCapacity();
		int getLeafCapacity();

		void setValuesMovedCallback(valuesMovedHandler callback);
		valuesMovedHandler getValuesMovedCallback();

		struct PathNode
		{
			PathNode(Branch* node, const int index) : Node(node), Index(index) {}
			PathNode(const PathNode& other) : Node(other.Node), Index(other.Index) {}
			Branch* Node;
			int Index;
		};

		struct Path
		{
			eastl::list<PathNode> Branches;
			Leaf* Leaf;
			int LeafIndex;
			bool Match;
		};

		Path GetPath(void* key);
		void Delete(Path& path);
		void Insert(Path& path, void* key, void* value);
		Path SeekToBegin();
		Path SeekToEnd();

		class iterator : public std::iterator<bidirectional_iterator_tag, void*>
		{
				BTree::Path _path;
				iterator(const BTree::Path& path) : _path(path) {}
			public:
				iterator(const iterator& iter) : _path(iter._path) {}

				bool MoveNext();
				bool MovePrior();
			
				iterator& operator++();
				iterator operator++(int);
				iterator& operator--();
				iterator operator--(int);
				bool operator==(const iterator& rhs);
				bool operator!=(const iterator& rhs);
				eastl::pair<void*,void*> operator*();
				bool End();
				bool Begin();

			friend class BTree;
		};

		iterator begin()
		{
			return iterator(SeekToBegin());
		}

		iterator end()
		{
			return iterator(SeekToEnd());
		}

		iterator find(void* key, bool& match)
		{
			Path p = GetPath(key);
			match = p.Match;
			return iterator(p);
		}


	private:
		Node* _root;
		int _branchCapacity;
		int _leafCapacity;
		ScalarType _keyType;
		ScalarType _valueType;
		
		void DoValuesMoved(Leaf* newLeaf);
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

		virtual void* GetValue(void* key, Leaf** leaf) = 0;
		virtual fs::wstring ToString() = 0;
		virtual void GetPath(void* key, BTree::Path& path) = 0;
		virtual void SeekToBegin(BTree::Path& path) = 0;
		virtual void SeekToEnd(BTree::Path& path) = 0;

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
		int IndexOf(void* key, bool& match);
		
		void InternalDelete(int index);

		//Index operations (for path)
		void InternalInsertIndex(int index, void* key, void* value);

	public:
		Leaf(BTree* tree);
		~Leaf();

		void* GetValue(void* key, Leaf** leaf);
		void* GetKey(function<bool(void*)>);
		fs::wstring ToString();
		void GetPath(void* key, BTree::Path& path);
		void SeekToBegin(BTree::Path& path);
		void SeekToEnd(BTree::Path& path);
		bool MoveNext(BTree::Path& path);
		bool MovePrior(BTree::Path& path);
		bool EndOfTree(BTree::Path& path);
		bool BeginOfTree(BTree::Path& path);

		//Index operations (for path -- behavior undefined for invalid paths)
		bool Delete(int index);
		Split* Insert(int index, void* key, void* value);
		

		eastl::pair<void*,void*> operator[](int);

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
			return iterator(&_values[_count * _tree->_valueType.Size], _tree->_valueType.Size);
		}
};

class Branch : public Node
{
	public:
		Branch(BTree* tree);
		Branch(BTree* tree, Node* left, Node* right, void* key);
		~Branch();

		void* GetValue(void* key, Leaf** leaf);
		fs::wstring ToString();	
		void GetPath(void* key, BTree::Path& path);
		void SeekToBegin(BTree::Path& path);
		void SeekToEnd(BTree::Path& path);
		bool MoveNext(BTree::Path& path);
		bool MovePrior(BTree::Path& path);

		//Index Operations (for path -- behavior not defined for invalid paths)
		bool Delete(int index);
		Split* Insert(int index, void* key, Node* child);

	private:
		Node** _children;
		int IndexOf(void* key);		
		void InternalInsertChild(int index, void* key, Node* child);

};


