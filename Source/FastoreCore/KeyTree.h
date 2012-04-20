#pragma once
#include "Schema\scalar.h"
#include "Schema\standardtypes.h"
#include "typedefs.h"
#include <functional>
#include "optional.h"
#include <EASTL\fixed_vector.h>
#include <sstream>

using namespace std;
using namespace standardtypes;

const int KeyDefaultListCapacity = 128;
const int KeyDefaultBranchListSize = 8;

struct KeySplit;
class KeyNode;

struct KeyInsertResult
{
	void* found;
	KeySplit* split;
};

struct KeyDeleteResult
{
	bool found;
	bool empty;
};

struct KeySplit
{
	void* key;
	KeyNode* right;
};

struct KeyTreeEntry
{
    fs::Key key;

    KeyTreeEntry();
    KeyTreeEntry(const fs::Key& k) : key(k) {};
    KeyTreeEntry(const KeyTreeEntry& entry) : key(entry.key) {};
};

class KeyTree
{
	public:
		KeyTree(ScalarType keyType);
		~KeyTree();

		fs::wstring ToString();
		int getListCapacity();

		struct PathNode
		{
			PathNode(KeyNode* node, const int index) : Node(node), Index(index) {}
			PathNode(const PathNode& other) : Node(other.Node), Index(other.Index) {}
			KeyNode* Node;
			int Index;
		};

		struct Path
		{
			eastl::fixed_vector<PathNode, KeyDefaultBranchListSize> Branches;
			KeyNode* Leaf;
			int LeafIndex;
			bool Match;
		};

		Path GetPath(void* key);
		void Delete(Path& path);
		//void Insert(Path& path, void* key, void* value);
		void Insert(Path& path, void* key);
		Path SeekToBegin();
		Path SeekToEnd();
		int Count()
		{
			return _count;
		}			

		class iterator : public std::iterator<bidirectional_iterator_tag, void*>
		{
				KeyTree::Path _path;
				iterator(const KeyTree::Path& path) : _path(path) {}
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
				KeyTreeEntry operator*();
				bool End();
				bool Begin();

			friend class KeyTree;
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
		KeyNode* _root;
		int _listCapacity;
		ScalarType _keyType;
		ScalarType _nodeType;
		int _count;

	friend class KeyNode;
	friend class KeyTree::iterator;
};

// WARNING: these type values must remain as optimizations use the numeric value
//Type 0 = Leaf;
//Type 1 = Branch;
//Todo: Enum

ScalarType GetKeyNodeType();

class KeyNode
{
	public:
		KeyNode(KeyTree* tree, int type = 0, int count = 0,  KeyNode* left = NULL, KeyNode* right = NULL, void* key = NULL) : _tree(tree), _count(count), _type(type)
		{
			try
			{
				_valueType = _tree->_nodeType;
				_keyType = _tree->_keyType;
				_keys = new char[(_tree->_listCapacity - type) * _keyType.Size];

				//Only the branches have value types, which are Links to other branches or leaves.
				if (_type == 1)
				{
					_values = new char[(_tree->_listCapacity) *  _valueType.Size];	
				}

				if(left != NULL)
				{
					memcpy(&_values[0], &left, _valueType.Size);
					memcpy(&_values[_valueType.Size], &right, _valueType.Size);
					memcpy(&_keys[0], key,_keyType.Size);
				}
			}
			catch(...)
			{
				delete[] _keys;

				if (_values != NULL)
				{
					delete[] _values;
				}
			}
		}

		~KeyNode() 
		{
			delete[] _keys;

			if (_values != NULL)
			{
				delete[] _values;
			}
		}

		int IndexOf(void* key, bool& match)
		{
 			auto result =_keyType.IndexOf(_keys, _count, key);
			match = result >= 0;
			return match ? result : ~result;
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

					result << i << ": " << (*(KeyNode**)&_values[_valueType.Size * i])->ToString();
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

					result <<_keyType.ToString(&_keys[i *_keyType.Size]);
				}
				result << "}";

				return result.str();
			}
		}

		void GetPath(void* key, KeyTree::Path& path)
		{
			if (_type == 1)
			{
				auto index = IndexOf(key);
				path.Branches.push_back(KeyTree::PathNode(this, index));
				(*(KeyNode**)&_values[index * _valueType.Size])->GetPath(key, path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = IndexOf(key, path.Match);
			}
		}

		void SeekToBegin(KeyTree::Path& path)
		{			
			if (_type == 1)
			{
				path.Branches.push_back(KeyTree::PathNode(this, 0));
				(*(KeyNode**)&_values[0])->SeekToBegin(path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = 0;
			}
		}

		void SeekToEnd(KeyTree::Path& path)
		{
			if (_type == 1)
			{
				path.Branches.push_back(KeyTree::PathNode(this, _count));
				(*(KeyNode**)&_values[_count * _valueType.Size])->SeekToEnd(path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = _count - 1;
			}
		}

		bool MoveNext(KeyTree::Path& path)
		{
			if (_type == 1)
			{
				KeyTree::PathNode& node = path.Branches.back();
				if (node.Index < _count)
				{
					++node.Index;
					(*(KeyNode**)(&node.Node->_values[node.Index * _valueType.Size]))->SeekToBegin(path);
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
						KeyTree::PathNode& node = path.Branches.back();

						if (node.Node->MoveNext(path))
							return true;
						else
							path.Branches.pop_back();
					}
					return false;
				}
			}
		}

		bool MovePrior(KeyTree::Path& path)
		{
			if (_type == 1)
			{
				KeyTree::PathNode& node = path.Branches.back();
				if (node.Index > 0)
				{
					--node.Index;
					(*(KeyNode**)(&node.Node->_values[node.Index * _valueType.Size]))->SeekToEnd(path);
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
						KeyTree::PathNode& node = path.Branches.back();

						if (node.Node->MovePrior(path))
							return true;
						else
							path.Branches.pop_back();
					}
					return false;
				}
			}
		}

		bool EndOfTree(KeyTree::Path& path)
		{
			return path.LeafIndex == path.Leaf->_count && path.Branches.size() == 0;
		}

		bool BeginOfTree(KeyTree::Path& path)
		{
			return path.LeafIndex < 0 && path.Branches.size() == 0;
		}

		//Index operations (for path -- behavior undefined for invalid paths)
		bool Delete(int index)
		{
			if (_type == 1)
			{
				delete *(KeyNode**)&_values[index * sizeof(KeyNode*)];
			}

			int size = _count - index;
			//Assumption -- Count > 0 (otherwise the key would not have been found)
			memmove(&_keys[(index) *_keyType.Size], &_keys[(index + 1) *_keyType.Size], size *_keyType.Size);
			if(_type == 1)
				memmove(&_values[index *_valueType.Size], &_values[(index + 1) *_valueType.Size], size * _valueType.Size);

			_count--;

			return _count + _type <= 0;			
		}

		KeySplit* Insert(int index, void* key, void* value)
		{
			if (_count != _tree->_listCapacity - _type)
			{
				InternalInsertIndex(index, key, value);
				return NULL;
			}
			else
			{
				KeyNode* node = new KeyNode(_tree);
				if (index != _count)
				{
					node->_count = (_tree->_listCapacity + 1) / 2;
					_count = _count - node->_count;

					memcpy(&node->_keys[0], &_keys[node->_count *_keyType.Size],  node->_count *_keyType.Size);
					if(_type == 1)
						memcpy(&node->_values[0], &_values[node->_count *_valueType.Size], node->_count * _valueType.Size);
				}

				if (index < _count)
					InternalInsertIndex(index, key, value);
				else
					node->InternalInsertIndex(index - _count, key, value);

				KeySplit* split = new KeySplit();
				split->key = &node->_keys[0];
				split->right = node;

				return split;
			}
		}

		KeySplit* Insert(int index, void* key, KeyNode* child)
		{
			if (_count != _tree->_listCapacity - _type)
			{
				InternalInsertIndex(index, key, &child);
				return NULL;
			}
			else
			{
				int mid = (_count + 1) / 2;
				KeyNode* node = new KeyNode(_tree, 1);
				node->_count = _count - mid;
				memcpy(&node->_keys[0], &_keys[mid *_keyType.Size], node->_count *_keyType.Size);
				if(_type == 1)
					memcpy(&node->_values[0], &_values[mid *_valueType.Size], (node->_count + 1) * _valueType.Size);
		
				_count = mid - 1;

				KeySplit* split = new KeySplit();
				split->key = &_keys[(mid - 1) *_keyType.Size];
				split->right = node;

				if (index <= _count)
					InternalInsertIndex(index, key, &child);
				else
					node->InternalInsertIndex(index - (_count + 1), key, &child);

				return split;
			}
		}


		KeyTreeEntry operator[](int index)
		{
			return KeyTreeEntry(_keys + (_tree->_keyType.Size * index));
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
			friend class KeyNode;
		};

		iterator valueBegin()
		{
			return iterator(_values,_valueType.Size);
		}

		iterator valueEnd()
		{
			return iterator(&_values[_count *_valueType.Size],_valueType.Size);
		}	

	private:
		int _type;
		int _count;
		char* _keys;
		char* _values;
		KeyTree* _tree;

		ScalarType _keyType;
		ScalarType _valueType;

		int IndexOf(void* key)
		{
			auto result =_keyType.IndexOf(_keys, _count, key);
			return result >= 0 ? result + 1 : ~result;
		}

		void InternalInsertIndex(int index, void* key, void* value)
		{
			int size = _count - index;
			if (_count != index)
			{
				memmove(&_keys[(index + 1) * _keyType.Size], &_keys[index  * _keyType.Size], size * _keyType.Size);
				if (_type == 1)
					memmove(&_values[(index + 1 + _type) * _valueType.Size], &_values[(index + _type) *_valueType.Size], size *_valueType.Size);
			}

			_keyType.CopyIn(key, &_keys[index *_keyType.Size]);
			if (_type == 1)
				_valueType.CopyIn(value, &_values[(index + _type) * _valueType.Size]);

			_count++;
		}

	friend class KeyTree::iterator;
};

