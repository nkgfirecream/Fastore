#pragma once
#include "Schema\scalar.h"
#include "Schema\standardtypes.h"
#include "typedefs.h"
#include <functional>
#include "optional.h"
#include <sstream>

using namespace std;
using namespace standardtypes;

struct Split;
class Node;
class BTree;

struct NodeType : public ScalarType
{
	NodeType();
};

struct NoOpNodeType : public ScalarType
{
	NoOpNodeType();
};

struct Split
{
	void* key;
	Node* right;
};

struct TreeEntry
{
    fs::Key key;
    fs::Value value;

    TreeEntry();
    TreeEntry(const fs::Key& k) : key(k) {};
    TreeEntry(const fs::Key& k, const fs::Value& v) : key(k), value(v) {};

    TreeEntry(const TreeEntry& entry) : key(entry.key), value(entry.value) {};
};

typedef function<void(void*,Node*)> valuesMovedHandler;

class BTree
{
	public:
		BTree(ScalarType keyType, ScalarType valueType);
		~BTree();

		fs::wstring ToString();
		int getListCapacity();

		const static short DefaultListCapacity = 128;

		void setValuesMovedCallback(valuesMovedHandler callback);
		valuesMovedHandler getValuesMovedCallback();

		struct PathNode
		{
			PathNode(Node* node, const short index) : Node(node), Index(index) {}
			PathNode(const PathNode& other) : Node(other.Node), Index(other.Index) {}
			Node* Node;
			short Index;
		};

		struct Path
		{
			std::vector<PathNode> Branches;
			Node* Leaf;
			short LeafIndex;
			bool Match;
		};

		Path GetPath(void* key);
		void Delete(Path& path);
		void Insert(Path& path, void* key, void* value);		

		class iterator : public std::iterator<bidirectional_iterator_tag, void*>
		{
				BTree::Path _path;
				iterator(const BTree::Path& path, bool eofOnEmpty);
			public:
				iterator(const iterator& iter) : _path(iter._path), _eof(iter._eof) {}				
			
				iterator& operator++();
				iterator operator++(int);
				iterator& operator--();
				iterator operator--(int);
				bool operator==(const iterator& rhs);
				bool operator!=(const iterator& rhs);
				TreeEntry operator*();

			private:
				void MoveNext();
				void MovePrior();
				bool _eof;

			friend class BTree;
		};

		iterator begin()
		{
			return iterator(SeekToFirst(), true);
		}

		//Best to cache this when using it.
		//The iterator will validate the path,
		//and the ++ operator will try to move the iterator
		//off the path. The iterator will search the path to ensure
		//there is nowhere else to go.
		iterator end()
		{
			return ++iterator(SeekToLast(), false);
		}

		//find either points to the item,
		//or points to the end.
		iterator find(void* key)
		{
			Path p = GetPath(key);
			
			if (p.Match)
				return iterator(p, false);
			else
				return end();
		}

		//find nearest points the to either the item, or the item direct AFTER it in the
		//BTrees sort order.
		iterator findNearest(void* key, bool& match)
		{
			Path p = GetPath(key);
			match = p.Match;

			return iterator(p, true);
		}

	private:
		Node* _root;
		ScalarType _keyType;
		ScalarType _valueType;
		ScalarType _nodeType;
		Path SeekToFirst();
		Path SeekToLast();
		
		void DoValuesMoved(Node* newLeaf);
		valuesMovedHandler _valuesMovedCallback;

	friend class Node;
	friend class BTree::iterator;
};

// WARNING: these type values must remain as optimizations use the numeric value
//Type 0 = Leaf;
//Type 1 = Branch;
//Todo: Enum
class Node
{
	public:
		Node(BTree* tree, short type = 0, short count = 0,  Node* left = NULL, Node* right = NULL, void* key = NULL) : _tree(tree), _count(count), _type(type)
		{
			try
			{
				_valueType = type == 1 ? _tree->_nodeType : _tree->_valueType;
				_keyType = _tree->_keyType;
				_keys = new char[(_tree->DefaultListCapacity - type) * _keyType.Size];
				_values = new char[(_tree->DefaultListCapacity) *  _valueType.Size];				

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
				delete[] _values;
			}
		}

		~Node() 
		{
			//TODO: Destructor should actually deallocate all values before deleting the array.
			delete[] _keys;
			delete[] _values;
		}

		short IndexOf(void* key, bool& match)
		{
 			auto result =_keyType.IndexOf(_keys, _count, key);
			match = result >= 0;
			return match ? result : ~result;
		}

		void* GetKey(function<bool(void*)> predicate)
		{
			for (short i = 0; i < _count; i++)
			{
				if (predicate(operator[](i).value))
					return operator[](i).key;
			}

			throw;
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

					result << i << ": " << (*(Node**)&_values[_valueType.Size * i])->ToString();
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
					result << ":";
					result << _tree->_valueType.ToString(&_values[i * _valueType.Size]);
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
				(*(Node**)&_values[index * _valueType.Size])->GetPath(key, path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = IndexOf(key, path.Match);
			}
		}

		void SeekToFirst(BTree::Path& path)
		{			
			if (_type == 1)
			{
				path.Branches.push_back(BTree::PathNode(this, 0));
				(*(Node**)&_values[0])->SeekToFirst(path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = 0;
			}
		}

		void SeekToLast(BTree::Path& path)
		{
			if (_type == 1)
			{
				path.Branches.push_back(BTree::PathNode(this, _count));
				(*(Node**)&_values[_count * _valueType.Size])->SeekToLast(path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = _count > 0 ? _count - 1 : 0;
			}
		}

		//Index operations (for path -- behavior undefined for invalid paths)
		bool Delete(short index)
		{	
			short size = _count - index;
			//Assumption -- Count > 0 (otherwise the key would not have been found)

			// Deallocate and shift keys
			_keyType.Deallocate(&_keys[index *_keyType.Size], 1);
			memmove(&_keys[(index) *_keyType.Size], &_keys[(index + 1) *_keyType.Size], size *_keyType.Size);

			// Deallocate and shift values
			_valueType.Deallocate(&_values[index + _type *_valueType.Size], 1);
			memmove(&_values[index + _type *_valueType.Size], &_values[(index + _type + 1) *_valueType.Size], size * _valueType.Size);

			_count--;

			return _count + _type <= 0;			
		}

		Split* Insert(short index, void* key, void* value)
		{
			if (_count != _tree->DefaultListCapacity - _type)
			{
				InternalInsertIndex(index, key, value);
				return NULL;
			}
			else
			{
				Node* node = new Node(_tree);
				if (index != _count)
				{
					node->_count = (_tree->DefaultListCapacity + 1) / 2;
					_count = _count - node->_count;

					memcpy(&node->_keys[0], &_keys[node->_count *_keyType.Size],  node->_count *_keyType.Size);
					memcpy(&node->_values[0], &_values[node->_count *_valueType.Size], node->_count * _valueType.Size);
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

		Split* Insert(short index, void* key, Node* child)
		{
			if (_count != _tree->DefaultListCapacity - _type)
			{
				InternalInsertIndex(index, key, &child);
				return NULL;
			}
			else
			{
				short mid = (_count + 1) / 2;
				Node* node = new Node(_tree, 1);
				node->_count = _count - mid;
				memcpy(&node->_keys[0], &_keys[mid *_keyType.Size], node->_count *_keyType.Size);
				memcpy(&node->_values[0], &_values[mid *_valueType.Size], (node->_count + 1) * _valueType.Size);
		
				_count = mid - 1;

				Split* split = new Split();
				split->key = node->GetChildKey();
				split->right = node;

				if (index <= _count)
					InternalInsertIndex(index, key, &child);
				else
					node->InternalInsertIndex(index - (_count + 1), key, &child);

				return split;
			}
		}

		void* GetChildKey()
		{
			if (_type == 1)
			{
				return (*(Node**)(&_values[0]))->GetChildKey();
			}
			else
			{
				return &_keys[0];
			}
		}

		TreeEntry operator[](short index)
		{
			return TreeEntry(_keys + (_tree->_keyType.Size * index), _values + (_tree->_valueType.Size * index));
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
			return iterator(_values,_valueType.Size);
		}

		iterator valueEnd()
		{
			return iterator(&_values[_count *_valueType.Size],_valueType.Size);
		}	

	private:
		short _type;
		short _count;
		char* _keys;
		char* _values;
		BTree* _tree;

		ScalarType _keyType;
		ScalarType _valueType;

		short IndexOf(void* key)
		{
			auto result =_keyType.IndexOf(_keys, _count, key);
			return result >= 0 ? result + 1 : ~result;
		}

		void InternalInsertIndex(short index, void* key, void* value)
		{
			int size = _count - index;
			if (_count != index)
			{
				memmove(&_keys[(index + 1) * _keyType.Size], &_keys[index  * _keyType.Size], size * _keyType.Size);
				memmove(&_values[(index + 1 + _type) * _valueType.Size], &_values[(index + _type) *_valueType.Size], size *_valueType.Size);
			}

			_keyType.CopyIn(key, &_keys[index *_keyType.Size]);
			_valueType.CopyIn(value, &_values[(index + _type) * _valueType.Size]);

			_count++;
		}

	friend class BTree::iterator;
};




