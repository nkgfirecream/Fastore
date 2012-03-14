#pragma once
#include "Schema\scalar.h"
#include "typedefs.h"
#include <functional>
#include "optional.h"
#include <EASTL\fixed_vector.h>
#include <sstream>

using namespace std;
const int DefaultListCapacity = 256;
const int DefaultBranchListSize = 8;

struct Split;
class Node;
class BTree;

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

typedef function<void(void*,Node*)> valuesMovedHandler;

class BTree
{
	public:
		BTree(ScalarType keyType, ScalarType valueType);
		~BTree();

		fs::wstring ToString();
		int getListCapacity();

		void setValuesMovedCallback(valuesMovedHandler callback);
		valuesMovedHandler getValuesMovedCallback();

		struct PathNode
		{
			PathNode(Node* node, const int index) : Node(node), Index(index) {}
			PathNode(const PathNode& other) : Node(other.Node), Index(other.Index) {}
			Node* Node;
			int Index;
		};

		struct Path
		{
			eastl::fixed_vector<PathNode, DefaultBranchListSize> Branches;
			Node* Leaf;
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
		int _listCapacity;
		ScalarType _keyType;
		ScalarType _valueType;
		
		void DoValuesMoved(Node* newLeaf);
		valuesMovedHandler _valuesMovedCallback;

	friend class Node;
	friend class BTree::iterator;
};

//Type 0 = Leaf;
//Type 1 = Branch;
//Todo: Enum
class Node
{
	public:
		Node(BTree* tree, int type = 0, int count = 0,  Node* left = NULL, Node* right = NULL, void* key = NULL) : _tree(tree), _count(count), _type(type)
		{
			try
			{
				_valueSize = type == 1 ? sizeof(Node*) : _tree->_valueType.Size;
				_keys = new char[(_tree->_listCapacity - type) * _tree->_keyType.Size];
				_values = new char[(_tree->_listCapacity) *  _valueSize];				

				if(left != NULL)
				{
					memcpy(&_values[0], &left, _valueSize);
					memcpy(&_values[_valueSize], &right, _valueSize);
					memcpy(&_keys[0], key, _tree->_keyType.Size);
				}
			}
			catch(...)
			{
				delete[] _keys;
				delete[] _values;
			}
		}

		~Node() 
		{
			delete[] _keys;
			delete[] _values;
		}

		int IndexOf(void* key, bool& match)
		{
 			auto result = _tree->_keyType.IndexOf(_keys, _count, key);
			match = result >= 0;
			return match ? result : ~result;
		}

		void* GetKey(function<bool(void*)> predicate)
		{
			for (int i = 0; i < _count; i++)
			{
				if (predicate(operator[](i).second))
					return operator[](i).first;
			}

			return NULL;
		}

		fs::wstring ToString()
		{
			if (_type == 1)
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

					result << i << ": " << (*(Node**)&_values[_valueSize * i])->ToString();
				}
				result << "]";

				return result.str();
			}
			else
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
					result << _tree->_valueType.ToString(&_values[i * _valueSize]);
				}
				result << "}";

				return result.str();
			}
		}

		void GetPath(void* key, BTree::Path& path)
		{
			if (_type == 1)
			{
				auto index = IndexOf(key);
				path.Branches.push_back(BTree::PathNode(this, index));
				(*(Node**)&_values[index * _valueSize])->GetPath(key, path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = IndexOf(key, path.Match);
			}
		}

		void SeekToBegin(BTree::Path& path)
		{			
			if (_type == 1)
			{
				path.Branches.push_back(BTree::PathNode(this, 0));
				(*(Node**)&_values[0])->SeekToBegin(path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = 0;
			}
		}

		void SeekToEnd(BTree::Path& path)
		{
			if (_type == 1)
			{
				path.Branches.push_back(BTree::PathNode(this, _count));
				(*(Node**)&_values[_count * _valueSize])->SeekToEnd(path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = _count -1;
			}
		}

		bool MoveNext(BTree::Path& path)
		{
			if (_type == 1)
			{
				BTree::PathNode& node = path.Branches.back();
				if (node.Index < _count)
				{
					++node.Index;
					(*(Node**)(&node.Node->_values[node.Index * _valueSize]))->SeekToBegin(path);
					return true;
				}
				return false;
			}
			else
			{
				++path.LeafIndex;
				if (path.LeafIndex < _count)
					return true;
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
		}

		bool MovePrior(BTree::Path& path)
		{
			if (_type == 1)
			{
				BTree::PathNode& node = path.Branches.back();
				if (node.Index > 0)
				{
					--node.Index;
					(*(Node**)(&node.Node->_values[node.Index * _valueSize]))->SeekToEnd(path);
					return true;
				}
				return false;
			}
			else
			{
				--path.LeafIndex;
				if (path.LeafIndex >= 0)
					return true;
				else
				{
					int depth =  - 1;
					// walk up until we are no longer at the beginning
					while (path.Branches.size() > 0)
					{
						BTree::PathNode& node = path.Branches.back();

						if (node.Node->MovePrior(path))
							return true;
						else
							path.Branches.pop_back();
					}
					return false;
				}
			}
		}

		bool EndOfTree(BTree::Path& path)
		{
			return path.LeafIndex == path.Leaf->_count && path.Branches.size() == 0;
		}

		bool BeginOfTree(BTree::Path& path)
		{
			return path.LeafIndex < 0 && path.Branches.size() == 0;
		}

		//Index operations (for path -- behavior undefined for invalid paths)
		bool Delete(int index)
		{
			if (_type == 1)
			{
				delete *(Node**)&_values[index * sizeof(Node*)];
			}

			int size = _count - index;
			//Assumption -- Count > 0 (otherwise the key would not have been found)
			memmove(&_keys[(index) * _tree->_keyType.Size], &_keys[(index + 1) * _tree->_keyType.Size], size * _tree->_keyType.Size);
			memmove(&_values[index *_valueSize], &_values[(index + 1) *_valueSize], size * _valueSize);

			_count--;

			return _count + _type <= 0;			
		}

		Split* Insert(int index, void* key, void* value)
		{
			if (_count != _tree->_listCapacity - _type)
			{
				InternalInsertIndex(index, key, value);
				return NULL;
			}
			else
			{
				Node* node = new Node(_tree);
				if (index != _count)
				{
					node->_count = (_tree->_listCapacity + 1) / 2;
					_count = _count - node->_count;

					memcpy(&node->_keys[0], &_keys[node->_count * _tree->_keyType.Size],  node->_count * _tree->_keyType.Size);
					memcpy(&node->_values[0], &_values[node->_count *_valueSize], node->_count * _valueSize);
				}

				if (index < _count)
					InternalInsertIndex(index, key, value);
				else
					node->InternalInsertIndex(index - _count, key, value);

				_tree->DoValuesMoved(node);

				Split* split = new Split();
				split->key = &node->_keys[0];
				split->right = node;

				return split;
			}
		}

		Split* Insert(int index, void* key, Node* child)
		{
			if (_count != _tree->_listCapacity - _type)
			{
				InternalInsertIndex(index, key, &child);
				return NULL;
			}
			else
			{
				int mid = (_count + 1) / 2;
				Node* node = new Node(_tree, 1);
				node->_count = _count - mid;
				memcpy(&node->_keys[0], &_keys[mid * _tree->_keyType.Size], node->_count * _tree->_keyType.Size);
				memcpy(&node->_values[0], &_values[mid *_valueSize], (node->_count + 1) * _valueSize);
		
				_count = mid - 1;

				Split* split = new Split();
				split->key = &_keys[(mid - 1) * _tree->_keyType.Size];
				split->right = node;

				if (index <= _count)
					InternalInsertIndex(index, key, &child);
				else
					node->InternalInsertIndex(index - (_count + 1), key, &child);

				return split;
			}
		}


		eastl::pair<void*,void*> operator[](int index)
		{
			return eastl::pair<void*,void*>(_keys + (_tree->_keyType.Size * index), _values + (_tree->_valueType.Size * index));
		}

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
			friend class Node;
		};

		iterator valueBegin()
		{
			return iterator(_values, _tree->_valueType.Size);
		}

		iterator valueEnd()
		{
			return iterator(&_values[_count * _tree->_valueType.Size], _tree->_valueType.Size);
		}	

	private:
		int _type;
		int _count;
		int _valueSize;
		char* _keys;
		char* _values;
		BTree* _tree;

		int IndexOf(void* key)
		{
			auto result = _tree->_keyType.IndexOf(_keys, _count, key);
			return result >= 0 ? result + 1 : ~result;
		}

		void InternalInsertIndex(int index, void* key, void* value)
		{
			int size = _count - index;
			if (_count != index)
			{
				memmove(&_keys[(index + 1) * _tree->_keyType.Size], &_keys[index  * _tree->_keyType.Size], size * _tree->_keyType.Size);
				memmove(&_values[(index + 1 + _type) * _valueSize], &_values[(index + _type) *_valueSize], size *_valueSize);
			}

			memcpy(&_keys[index * _tree->_keyType.Size], key, _tree->_keyType.Size);
			memcpy(&_values[(index + _type) * _valueSize], value, _valueSize);

			_count++;
		}

	friend class BTree::iterator;
};


