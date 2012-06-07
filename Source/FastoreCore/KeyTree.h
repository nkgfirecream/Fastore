#pragma once
#include "Schema\scalar.h"
#include "Schema\standardtypes.h"
#include "typedefs.h"
#include "treeentry.h"
#include <functional>
#include <array>
#include "optional.h"
#include <sstream>

using namespace std;
using namespace standardtypes;

struct KeySplit;
class KeyNode;

struct KeyNodeType : public ScalarType
{
	KeyNodeType();
};

struct NoOpKeyNodeType : public ScalarType
{
	NoOpKeyNodeType();
};

struct KeySplit
{
	void* key;
	KeyNode* right;
};

class KeyTree
{
	public:
		KeyTree(ScalarType keyType);
		~KeyTree();

		const static short DefaultListCapacity = 64;

		fs::wstring ToString();
		int getListCapacity();

		struct PathNode
		{
			PathNode(KeyNode* node, const short index) : Node(node), Index(index) {}
			PathNode(const PathNode& other) : Node(other.Node), Index(other.Index) {}
			KeyNode* Node;
			short Index;
		};

		struct Path
		{
			std::vector<PathNode> Branches;
			KeyNode* Leaf;
			short LeafIndex;
			bool Match;
		};

		Path GetPath(void* key);
		void Delete(Path& path);
		//void Insert(Path& path, void* key, void* value);
		void Insert(Path& path, void* key);
		Path SeekToFirst();
		Path SeekToLast();
		int Count()
		{
			return _count;
		}			

		class iterator : public std::iterator<bidirectional_iterator_tag, void*>
		{
				KeyTree::Path _path;
				iterator(const KeyTree::Path& path, bool eofOnEmpty);
			public:
				iterator(const iterator& iter) : _path(iter._path), _eof(iter._eof) {}

				
				bool TestPath();
			
				iterator& operator++();
				iterator operator++(int);
				iterator& operator--();
				iterator operator--(int);
				bool operator==(const iterator& rhs);
				bool operator!=(const iterator& rhs);
				TreeEntry operator*();

			private:
				bool MoveNext();
				bool MovePrior();
				bool _eof;

			friend class KeyTree;
		};

		iterator begin()
		{
			return iterator(SeekToFirst(), true);
		}

		iterator end()
		{
			return ++iterator(SeekToLast(), false);
		}

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
		KeyNode* _root;
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
class KeyNode
{
	public:
		KeyNode(KeyTree* tree, short type = 0, short count = 0,  KeyNode* left = NULL, KeyNode* right = NULL, void* key = NULL) : _tree(tree), _count(count), _type(type)
		{
			try
			{
				_valueType = _tree->_nodeType;
				_keyType = _tree->_keyType;
				_keys = new char[(_tree->DefaultListCapacity - type) * _keyType.Size];

				//Only the branches have value types, which are Links to other branches or leaves.
				if (_type == 1)
				{
					_values = new char[(_tree->DefaultListCapacity) *  _valueType.Size];	
				}
				else
				{
					_values = NULL;
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

		short IndexOf(void* key, bool& match)
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

		void SeekToFirst(KeyTree::Path& path)
		{			
			if (_type == 1)
			{
				path.Branches.push_back(KeyTree::PathNode(this, 0));
				(*(KeyNode**)&_values[0])->SeekToFirst(path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = 0;
			}
		}

		void SeekToLast(KeyTree::Path& path)
		{
			if (_type == 1)
			{
				path.Branches.push_back(KeyTree::PathNode(this, _count));
				(*(KeyNode**)&_values[_count * _valueType.Size])->SeekToLast(path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = _count > 0 ? _count - 1 : 0;
			}
		}

		//Index operations (for path -- behavior undefined for invalid paths)
		//Valid for leaves only (type 0)
		bool Delete(short index)
		{
			short size = _count - index;
			//Assumption -- Count > 0 (otherwise the key would not have been found)

			// Deallocate and shift keys
			_keyType.Deallocate(&_keys[index *_keyType.Size], 1);
			memmove(&_keys[index *_keyType.Size], &_keys[(index + 1) *_keyType.Size], size *_keyType.Size);

			_count--;

			return _count <= 0;				
		}

		KeySplit* Insert(short index, void* key, void* value)
		{
			if (_count != _tree->DefaultListCapacity - _type)
			{
				InternalInsertIndex(index, key, value);
				return NULL;
			}
			else
			{
				KeyNode* node = new KeyNode(_tree);
				if (index != _count)
				{
					node->_count = (_tree->DefaultListCapacity + 1) / 2;
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

		KeySplit* Insert(short index, void* key, KeyNode* child)
		{
			if (_count != _tree->DefaultListCapacity - _type)
			{
				InternalInsertIndex(index, key, &child);
				return NULL;
			}
			else
			{
				short mid = (_count + 1) / 2;
				KeyNode* node = new KeyNode(_tree, 1);
				node->_count = _count - mid;
				memcpy(&node->_keys[0], &_keys[mid *_keyType.Size], node->_count *_keyType.Size);
				memcpy(&node->_values[0], &_values[mid *_valueType.Size], (node->_count + 1) * _valueType.Size);
		
				_count = mid - 1;

				KeySplit* split = new KeySplit();
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
				return (*(KeyNode**)(&_values[0]))->GetChildKey();
			}
			else
			{
				return &_keys[0];
			}
	   }

	   //Should only be called on type 1s
		void UpdateKey(short index, KeyTree::Path& path, int depth)
		{
			auto pathNode = path.Branches.at(depth);
			if (index != 0)
			{
				KeyNode* node = *(KeyNode**)&_values[index * _valueType.Size];
				memcpy(&_keys[(index - 1) * _keyType.Size], node->GetChildKey(), _keyType.Size);
			}
			else if (depth != 0)
			{
				pathNode.Node->UpdateKey(pathNode.Index, path, depth - 1);
			}

			//Do nothing, we don't update the the first index on the root
		}


		KeyNode* RebalanceLeaf(KeyTree::Path& path, int depth)
		{
				//No need to rebalance
				if (_count >= _tree->DefaultListCapacity / 2 || depth == 0)
					return NULL;

				KeyNode* parent = path.Branches.at(depth - 1).Node;
				short pIndex = path.Branches.at(depth - 1).Index;
				KeyNode* rightSib = pIndex < parent->_count ? *(KeyNode**)((*parent)[pIndex + 1].value) : NULL;
				KeyNode* leftSib = pIndex > 0 ? *(KeyNode**)((*parent)[pIndex - 1].value) : NULL;

				//Attempt to borrow from right sibling
				if (rightSib != NULL && rightSib->_count > _tree->DefaultListCapacity / 2)
				{
					//Grab left item from right sibling
					memcpy(&_keys[_count * _keyType.Size], &rightSib->_keys[0], _keyType.Size);
					_count++;

					//Shift right sibling's values down.
					memcpy(&rightSib->_keys[0], &rightSib->_keys[_keyType.Size], _keyType.Size * (rightSib->_count - 1));
					rightSib->_count--;

					//Recursively update parent separator on right with sibling's new left.
					//(If the parent's separator is the left most separator its parent also needs to be updated)
					parent->UpdateKey(pIndex + 1, path, depth - 1);
					return NULL;
				}

				//Attempt to borrow from left sibling
				if (leftSib != NULL && leftSib->_count > _tree->DefaultListCapacity / 2)
				{
					//Make room for new item
					memcpy(&_keys[_keyType.Size], &_keys[0], _count * _keyType.Size);

					//Grab right item from left sibling.
					memcpy(&_keys[0], &leftSib->_keys[(leftSib->_count - 1) * _keyType.Size], _keyType.Size);
					_count++;

					//Delete right item from left sibling
					leftSib->_count--;

					//Recursively update parent separator on left with our new left
					//(If the parent's separator is the left most separator its parent also needs to be updated)
					parent->UpdateKey(pIndex, path, depth - 1);
					return NULL;
				}

				//Attempt to merge with right sibling
				if (rightSib != NULL && rightSib->_count + this->_count <= _tree->DefaultListCapacity)
				{
					//Grab all of right's items
					memcpy(&_keys[_count * _keyType.Size], &rightSib ->_keys[0], rightSib ->_count * _keyType.Size);
					_count += rightSib->_count;

					//Delete right sibling
					delete rightSib;

					//Delete parent separator on right
					parent->BranchDelete(pIndex);

					//In case we deleted the left most item,
					//update the keys
					if (pIndex == 0)
						parent->UpdateKey(0, path, depth - 1);

					//Rebalance parent
					return parent->RebalanceBranch(path, depth - 1);
				}

				//Attempt to merge with left sibling
				if (leftSib != NULL && leftSib->_count + this->_count <= _tree->DefaultListCapacity)
				{
					//Move everything over to left.
					memcpy(&leftSib->_keys[leftSib->_count * _keyType.Size], &_keys[0], _count * _keyType.Size);
					leftSib->_count += _count;

					//Suicide
					delete this;

					//Delete parent separator on left
					parent->BranchDelete(pIndex - 1);
					
					//Rebalance parent
					return parent->RebalanceBranch(path, depth - 1);
				}

				//If everything failed, something is wrong
				throw "Leaf rebalancing failed";
		}

		KeyNode* RebalanceBranch(KeyTree::Path& path, int depth)
		{
			//Only one item, we are root, replace root with child
				if (depth == 0 && _count == 0)
				{
					KeyNode* root = *(KeyNode**)_values;
					//Questionable...
					delete this;
					return root;
				}

				//No need to rebalance if we are an acceptable size
				if ((_count + 1) >= _tree->DefaultListCapacity / 2 || depth == 0)
					return NULL;				

				//At this point we know we are not root, so we have a parent.
				KeyNode* parent = path.Branches.at(depth - 1).Node;
				short pIndex = path.Branches.at(depth - 1).Index;
				KeyNode* rightSib = pIndex < parent->_count ? *(KeyNode**)((*parent)[pIndex + 1].value) : NULL;
				KeyNode* leftSib = pIndex > 0 ? *(KeyNode**)((*parent)[pIndex - 1].value) : NULL;

				//Attempt to borrow from right sibling
				if (rightSib != NULL && (rightSib->_count + 1) > _tree->DefaultListCapacity / 2)
				{
					//Grab left item from right sibling		
					memcpy(&_values[(_count + 1) * _valueType.Size], &rightSib->_values[0], _valueType.Size);
					memcpy(&_keys[_count * _keyType.Size],  (*(KeyNode**)&rightSib->_values[0])->GetChildKey(), _keyType.Size);
					_count++;

					memcpy(&rightSib->_keys[0], &rightSib->_keys[_keyType.Size], _keyType.Size * (rightSib->_count - 1));
					memcpy(&rightSib->_values[0], &rightSib->_values[_valueType.Size], _valueType.Size * (rightSib->_count));
					rightSib->_count--;

					//Recursively update parent separator on right with sibling's new left.
					//(If the parent's separator is the left most separator its parent also needs to be updated)
					parent->UpdateKey(pIndex + 1, path, depth - 1);

					return NULL;
				}

				//Attempt to borrow form left sibling
				if (leftSib != NULL && (leftSib->_count + 1) > _tree->DefaultListCapacity / 2)
				{
						//Make room for new item
					memcpy(&_keys[_keyType.Size], &_keys[0], _count * _keyType.Size);
					memcpy(&_values[_valueType.Size], &_values[0], _count * _valueType.Size);

					//Grab right item from left sibling.
					memcpy(&_values[0], &leftSib->_values[leftSib->_count * _valueType.Size], _valueType.Size);
					memcpy(&_keys[0],  (*(KeyNode**)_values[1 * _valueType.Size])->GetChildKey(), _keyType.Size);
					_count++;

					leftSib->_count--;

					//Recursively update parent separator on left with our new left
					//(If the parent's separator is the left most separator its parent 
					parent->UpdateKey(pIndex, path, depth - 1);
					return NULL;
				}

				//Attempt to merge with right sibling
				if (rightSib != NULL && (rightSib->_count + 1 + this->_count + 1) <= _tree->DefaultListCapacity)
				{
					//Grab all of rights items
					memcpy(&_keys[(_count + 1) * _keyType.Size], &rightSib->_keys[0], rightSib->_count * _keyType.Size);
					memcpy(&_values[(_count + 1) * _valueType.Size], &rightSib->_values[0], (rightSib->_count + 1) *_valueType.Size);

					//Create new separator
					memcpy(&_keys[(_count) * _keyType.Size], (*(KeyNode**)&rightSib->_values[0])->GetChildKey(), _keyType.Size);
					_count += (rightSib->_count + 1); //Count is the number of separators. + 1 is for the the one we are taking from parent.
					
					delete rightSib;

					//Delete parent separator on right
					parent->BranchDelete(pIndex);

					//In case we deleted the left most item,
					//update the keys
					if (pIndex == 0)
						parent->UpdateKey(0, path, depth - 1);

					//Rebalance parent
					return parent->RebalanceBranch(path, depth - 1);
				}

				//Attempt to merge with left sibling
				if (leftSib != NULL && (leftSib->_count + 1 + this->_count + 1) <= _tree->DefaultListCapacity)
				{
					//Move everything over to left.
					memcpy(&leftSib->_keys[(leftSib->_count + 1) * _keyType.Size], &_keys[0], _count * _keyType.Size);
					memcpy(&leftSib->_values[(leftSib->_count + 1) * _valueType.Size], &_values[0], _count *_valueType.Size);					
					
					//create new separator
					memcpy(&leftSib->_keys[(leftSib->_count) * _keyType.Size], (*(KeyNode**)&_values[0])->GetChildKey(), _keyType.Size);
					leftSib->_count += (_count + 1); //Count is the number of separators. + 1 is for the the one we are taking from parent.

					//Suicide
					delete this;

					//Delete parent separator on left
					parent->BranchDelete(pIndex - 1);
					
					//Rebalance parent
					return parent->RebalanceBranch(path, depth - 1);
				}

				//If everything failed, something is wrong
				throw "Branch rebalancing failed";
		}

		//This is a special case delete from branches.
		//It merely shift memory around, and does not deallocate anything because it does
		//not know if it was called as the result of a merge
		//Index is the KEY index, not the item index
		void BranchDelete(short index)
		{
			int size = _count - index;
			memcpy(&_keys[index * _keyType.Size], &_keys[(index + 1) * _keyType.Size],  size * _keyType.Size);
			memcpy(&_values[(index + 1) * _valueType.Size], &_values[(index + 2) * _valueType.Size],  size * _valueType.Size);

			_count--;
		}


		TreeEntry operator[](short index)
		{
			return TreeEntry(_keys + (_tree->_keyType.Size * index), _values + (_valueType.Size * index));
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
		short _type;
		short _count;
		char* _keys;
		char* _values;
		KeyTree* _tree;

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

