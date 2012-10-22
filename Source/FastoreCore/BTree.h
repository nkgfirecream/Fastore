#pragma once
#include "Schema/scalar.h"
#include "Schema/standardtypes.h"

#include "treeentry.h"
#include <functional>
#include <sstream>
#include <vector>
#include <limits>
#include <stdexcept>

#include <cassert>
#include <cstring>

using namespace std;
using namespace standardtypes;

struct Split;
class Node;
class BTree;

struct Split
{
	void* key;
	Node* right;
};

typedef function<void(void*,Node*)> onevaluesMovedHandler;

class BTree
{
	public:
		BTree(const ScalarType& keyType, const ScalarType& valueType);
		BTree(const ScalarType& keyType);
		~BTree();

		std::wstring ToString();

		const static short DefaultListCapacity = 128;

		void setValuesMovedCallback(onevaluesMovedHandler callback);
		onevaluesMovedHandler getValuesMovedCallback();

		struct PathNode
		{
			PathNode(Node* node, const short index) : pNode(node), Index(index) {}
			PathNode(const PathNode& other) : pNode(other.pNode), Index(other.Index) {}
			PathNode() : pNode(nullptr), Index(0) {}
			Node* pNode;
			short Index;
		};

		const static std::vector<PathNode>::size_type DefaultPathCapacity = 8;

		struct Path
		{
			Path() : Branches() 
			{
				Branches.reserve(DefaultPathCapacity);
			}
			std::vector<PathNode> Branches;
			Node* Leaf;
			short LeafIndex;
			bool Match;
		};

		void GetPath(void* key, Path& result);
		void Delete(Path& path);
		void Insert(Path& path, void* key, void* value);
		int Count();

		class iterator : public std::iterator<bidirectional_iterator_tag, void*>
		{			
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
				BTree::Path _path;
				iterator(const BTree::Path& path);
				void MoveNext();
				void MovePrior();
				bool _eof;

			friend class BTree;
		};

		iterator begin()
		{
			return iterator(SeekToBegin());
		}

		//Best to cache this when using it.
		//The iterator will validate the path,
		//and the ++ operator will try to move the iterator
		//off the path. The iterator will search the path to ensure
		//there is nowhere else to go.
		iterator end()
		{
			return iterator(SeekToEnd());
		}

		//find either points to the item,
		//or points to the end.
		iterator find(void* key)
		{
			Path p;
			GetPath(key, p);
			
			if (p.Match)
				return iterator(p);
			else
				return end();
		}

		//find nearest points the to either the item, or the item directly AFTER it in the
		//BTrees sort order.
		iterator findNearest(void* key, bool& match)
		{
			Path p;
			GetPath(key, p);
			match = p.Match;

			return iterator(p);
		}

		Node* _root;

	private:
		bool _keyOnly;
		const ScalarType& _keyType;
		const ScalarType& _valueType;
		Path SeekToBegin();
		Path SeekToEnd();

		int _count;
		short _listCapacity;
		
		void DoValuesMoved(Node* newLeaf);
		onevaluesMovedHandler _valuesMovedCallback;

	friend class Node;
	friend class BTree::iterator;
};

class Node
{
	public:
		Node(BTree& tree, bool branch = false, short count = 0,  Node* left = NULL, Node* right = NULL, void* key = NULL) : _tree(tree), _count(count), _branch(branch)
		{
			try
			{
				if(_branch)
				{
					_keys = new char[(_tree._listCapacity - 1) * _tree._keyType.Size];
					_values = new char[(_tree._listCapacity) * sizeof(Node*)];		
				}
				else
				{
					_keys = new char[(_tree._listCapacity) * _tree._keyType.Size];
					if (!_tree._keyOnly)
					{				
						_values = new char[(_tree._listCapacity) * _tree._valueType.Size];		
					}
					else
					{
						_values = NULL;
					}	
				}	

				if(left != NULL)
				{
					memcpy(&_values[0], &left, sizeof(Node*));
					memcpy(&_values[sizeof(Node*)], &right, sizeof(Node*));
					memcpy(&_keys[0], key, _tree._keyType.Size);
				}
			}
			catch(...)
			{
				delete[] _keys;
				if (!_tree._keyOnly)
					delete[] _values;
			}
		}

		~Node() 
		{
			//When destroying Nodes, we must consider whether we are a leaf or a branch.
			//A leaf must destroy all the keys and values, while a branch must destroy only the values.
			//The reason is that the keys in the branch are actually copied up from the branches.	
			if (_branch)
			{
				standardtypes::StandardNodeType.Deallocate(_values, _count + 1);
			}
			else
			{
				if (!_tree._keyOnly)
				{
					_tree._valueType.Deallocate(_values, _count);
					delete[] _values;
				}

				_tree._keyType.Deallocate(_keys, _count);
				delete[] _keys;
			}			
		}

		short IndexOf(void* key, bool& match)
		{
		  short n = short(_tree._keyType.IndexOf(_keys, _count, key));

		  assert(     -std::numeric_limits<short>::max() <= n );
		  assert( n <= std::numeric_limits<short>::max() );

		  match = n >= 0;

		  return match ? n : short(~n);
		}

		void* GetKey(function<bool(void*)> predicate)
		{
			for (short i = 0; i < _count; ++i)
			{
				if (predicate(operator[](i).value))
					return operator[](i).key;
			}

			throw "Value not found in Node!";
		}

		std::wstring ToString()
		{
			if (_branch)
			{
				wstringstream result;

				result << "\n[";
				bool first = true;
				for (int i = 0; i <= _count; ++i)
				{
					if (!first)
						result << ",";
					else
						first = false;

					result << i << ": " << (*(Node**)&_values[sizeof(Node*) * i])->ToString();
				}
				result << "]";

				return result.str();
			}
			else
			{
				wstringstream result;
				result << "\n{";
				bool first = true;
				for(int i = 0; i < _count; ++i)
				{
					if(!first)
						result << ",";
					else
						first = false;

					result << _tree._keyType.ToString(&_keys[i * _tree._keyType.Size]);
					result << ":";
					result << _tree._valueType.ToString(&_values[i * _tree._valueType.Size]);
				}
				result << "}";

				return result.str();
			}
		}

		void GetPath(void* key, BTree::Path& path)
		{
			if (_branch)
			{
				auto index = IndexOf(key);
				path.Branches.push_back(BTree::PathNode(this, index));
				(*(Node**)&_values[index * sizeof(Node*)])->GetPath(key, path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = IndexOf(key, path.Match);
			}
		}

		void SeekToFirst(BTree::Path& path)
		{			
			if (_branch)
			{
				path.Branches.push_back(BTree::PathNode(this, 0));
				(*(Node**)&_values[0])->SeekToFirst(path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = 0;
				path.Match = false;
			}
		}

		void SeekToLast(BTree::Path& path)
		{
			if (_branch)
			{
				path.Branches.push_back(BTree::PathNode(this, _count));
				(*(Node**)&_values[_count * sizeof(Node*)])->SeekToLast(path);
			}
			else
			{
				path.Leaf = this;
				path.LeafIndex = short(_count > 0 ? _count - 1 : 0);
				path.Match = false;
			}
		}

		void* GetChildKey()
		{
			if (_branch)
			{
				return (*(Node**)(&_values[0]))->GetChildKey();
			}
			else
			{
				return &_keys[0];
			}
		}


		//Index operations (for path -- behavior undefined for invalid paths)
		//Only valid for leaves!
		bool Delete(short index)
		{	
			const int size = _count - index - 1;
			//Assumption -- Count > 0 (otherwise the key would not have been found)

			// Deallocate and shift keys
			_tree._keyType.Deallocate(&_keys[index * _tree._keyType.Size], 1);
			memmove(&_keys[index * _tree._keyType.Size], &_keys[(index + 1) * _tree._keyType.Size], size * _tree._keyType.Size);

			// Deallocate and shift values
			if (!_tree._keyOnly)
			{
				_tree._valueType.Deallocate(&_values[index *	_tree._valueType.Size], 1);
				memmove(&_values[index * _tree._valueType.Size], &_values[(index + 1) * _tree._valueType.Size], size * _tree._valueType.Size);
			}

			_count--;

			return _count <= 0;			
		}

		Split* Insert(short index, void* key, void* value)
		{
			if (_count != _tree._listCapacity)
			{
			    InternalInsertIndex(index, key, value);
			    return NULL;
			}

			Node* node = new Node(_tree, false);
			if (!_tree._keyOnly)
			{
			    node->_count = (_tree._listCapacity + 1) / 2;
			    _count = short(_count - node->_count);

			    memcpy( node->_keys, 
				    _keys + node->_count * _tree._keyType.Size,
				    node->_count * _tree._keyType.Size );

			    if (!_tree._keyOnly)
			      memcpy( node->_values, 
				      _values + node->_count * _tree._valueType.Size, 
				      node->_count * _tree._valueType.Size );
			}

			if (index < _count)
			  InternalInsertIndex(index, key, value);
			else
			  node->InternalInsertIndex(short(index - _count), key, value);


			_tree.DoValuesMoved(node);

			Split* split = new Split();
			split->key = node->_keys;
			split->right = node;

			return split;
		}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

		Split* Insert(short index, void* key, Node* child)
		{
			if (_count != _tree._listCapacity - 1)
			{
				InternalInsertIndex(index, key, child);
				return NULL;
			}
			else
			{
				short mid = (_count + 1) / 2;
				Node* node = new Node(_tree, true);
				node->_count = _count - mid;
				memcpy(&node->_keys[0], &_keys[mid * _tree._keyType.Size], node->_count * _tree._keyType.Size);
				memcpy(&node->_values[0], &_values[mid * sizeof(Node*)], (node->_count + 1) * sizeof(Node*));
		
				_count = mid - 1;

				Split* split = new Split();
				split->key = node->GetChildKey();
				split->right = node;

				if (index <= _count)
					InternalInsertIndex(index, key, child);
				else
					node->InternalInsertIndex(index - (_count + 1), key, child);

				return split;
			}
		}		

		TreeEntry operator[](short index)
		{
			return TreeEntry(_keys + (_tree._keyType.Size * index), _values + (_tree._valueType.Size * index));
		}

		//Should only be called on branches
		void UpdateKey(short index, BTree::Path& path, size_t depth)
		{
			auto pathNode = path.Branches.at(depth);
			if (index != 0)
			{
				Node* node = *(Node**)&_values[index * sizeof(Node*)];
				memcpy(&_keys[(index - 1) *  _tree._keyType.Size], node->GetChildKey(),  _tree._keyType.Size);
			}
			else if (depth != 0)
			{
				pathNode.pNode->UpdateKey(pathNode.Index, path, depth - 1);
			}

			//Do nothing, we don't update the the first index on the root
		}


		Node* RebalanceLeaf(BTree::Path& path, size_t depth)
		{
				//No need to rebalance
				if (_count >= _tree._listCapacity / 2 || depth == 0)
					return NULL;

				Node* parent = path.Branches.at(depth - 1).pNode;
				short pIndex = path.Branches.at(depth - 1).Index;
				Node* rightSib = pIndex < parent->_count ? *(Node**)(parent->_values + ((pIndex + 1) * sizeof(Node*))) : NULL;
				Node* leftSib = pIndex > 0 ? *(Node**)(parent->_values + ((pIndex - 1) * sizeof(Node*))) : NULL;

				//Attempt to borrow from right sibling
				if (rightSib != NULL && rightSib->_count > _tree._listCapacity / 2)
				{
					//Grab left item from right sibling
					memcpy(&_keys[_count *  _tree._keyType.Size], &rightSib->_keys[0], _tree._keyType.Size);
					memcpy(&_values[_count * _tree._valueType.Size], &rightSib->_values[0], _tree._valueType.Size);
					_count++;

					//Shift right sibling's values down.
					memcpy(&rightSib->_keys[0], &rightSib->_keys[_tree._keyType.Size], _tree._keyType.Size * (rightSib->_count - 1));
					memcpy(&rightSib->_values[0], &rightSib->_values[_tree._valueType.Size], _tree._valueType.Size * (rightSib->_count - 1));
					rightSib->_count--;

					//Recursively update parent separator on right with sibling's new left.
					//(If the parent's separator is the left most separator its parent also needs to be updated)
					parent->UpdateKey(pIndex + 1, path, depth - 1);
					return NULL;
				}

				//Attempt to borrow from left sibling
				if (leftSib != NULL && leftSib->_count > _tree._listCapacity / 2)
				{
					//Make room for new item
					memcpy(&_keys[ _tree._keyType.Size], &_keys[0], _count *  _tree._keyType.Size);
					memcpy(&_values[_tree._valueType.Size], &_values[0], _count * _tree._valueType.Size);

					//Grab right item from left sibling.
					memcpy(&_keys[0], &leftSib->_keys[(leftSib->_count - 1) * _tree._keyType.Size], _tree._keyType.Size);
					memcpy(&_values[0], &leftSib->_values[(leftSib->_count - 1) * _tree._valueType.Size], _tree._valueType.Size);
					_count++;

					//Delete right item from left sibling
					leftSib->_count--;

					//Recursively update parent separator on left with our new left
					//(If the parent's separator is the left most separator its parent also needs to be updated)
					parent->UpdateKey(pIndex, path, depth - 1);
					return NULL;
				}

				//Attempt to merge with right sibling
				if (rightSib != NULL && rightSib->_count + this->_count <= _tree._listCapacity)
				{
					//Grab all of right's items
					memcpy(&_keys[_count *  _tree._keyType.Size], &rightSib ->_keys[0], rightSib ->_count *  _tree._keyType.Size);
					memcpy(&_values[_count * _tree._valueType.Size], &rightSib ->_values[0], rightSib ->_count * _tree._valueType.Size);
					_count += rightSib->_count;

					//Delete right sibling
					rightSib->_count = -1;
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
				if (leftSib != NULL && leftSib->_count + this->_count <= _tree._listCapacity)
				{
					//Move everything over to left.
					memcpy(&leftSib->_keys[leftSib->_count *  _tree._keyType.Size], &_keys[0], _count *  _tree._keyType.Size);
					memcpy(&leftSib->_values[leftSib->_count * _tree._valueType.Size], &_values[0], _count * _tree._valueType.Size);
					leftSib->_count += _count;

					//Suicide
					_count = -1;
					delete this;

					//Delete parent separator on left
					parent->BranchDelete(pIndex - 1);
					
					//Rebalance parent
					return parent->RebalanceBranch(path, depth - 1);
				}

				//If everything failed, something is wrong
				throw "Leaf rebalancing failed";
		}

		Node* RebalanceBranch(BTree::Path& path, size_t depth)
		{
			//Only one item, we are root, replace root with child
				if (depth == 0 && _count == 0)
				{
					Node* root = *(Node**)_values;
					//Questionable...
					this->_count = -1;
					delete this;
					return root;
				}

				//No need to rebalance if we are an acceptable size
				if ((_count + 1) >= _tree._listCapacity / 2 || depth == 0)
					return NULL;				

				//At this point we know we are not root, so we have a parent.
				Node* parent = path.Branches.at(depth - 1).pNode;
				short pIndex = path.Branches.at(depth - 1).Index;
				Node* rightSib = pIndex < parent->_count ? *(Node**)(parent->_values + ((pIndex + 1) * sizeof(Node*))) : NULL;
				Node* leftSib = pIndex > 0 ? *(Node**)(parent->_values + ((pIndex - 1) * sizeof(Node*))) : NULL;

				//Attempt to borrow from right sibling
				if (rightSib != NULL && (rightSib->_count + 1) > _tree._listCapacity / 2)
				{
					//Grab left item from right sibling		
					memcpy(&_values[(_count + 1) * sizeof(Node*)], &rightSib->_values[0], sizeof(Node*));
					memcpy(&_keys[_count *  _tree._keyType.Size],  (*(Node**)&rightSib->_values[0])->GetChildKey(),  _tree._keyType.Size);
					_count++;

					memcpy(&rightSib->_keys[0], &rightSib->_keys[ _tree._keyType.Size],  _tree._keyType.Size * (rightSib->_count - 1));
					memcpy(&rightSib->_values[0], &rightSib->_values[sizeof(Node*)], sizeof(Node*) * (rightSib->_count));
					rightSib->_count--;

					//Recursively update parent separator on right with sibling's new left.
					//(If the parent's separator is the left most separator its parent also needs to be updated)
					parent->UpdateKey(pIndex + 1, path, depth - 1);

					return NULL;
				}

				//Attempt to borrow form left sibling
				if (leftSib != NULL && (leftSib->_count + 1) > _tree._listCapacity / 2)
				{
						//Make room for new item
					memcpy(&_keys[ _tree._keyType.Size], &_keys[0], _count *  _tree._keyType.Size);
					memcpy(&_values[sizeof(Node*)], &_values[0], _count * sizeof(Node*));

					//Grab right item from left sibling.
					memcpy(&_values[0], &leftSib->_values[leftSib->_count * sizeof(Node*)], sizeof(Node*));
					Node *v( reinterpret_cast<Node*>(_values + sizeof(Node*)) );
					memcpy(&_keys[0],  v->GetChildKey(), _tree._keyType.Size);
					_count++;

					leftSib->_count--;

					//Recursively update parent separator on left with our new left
					//(If the parent's separator is the left most separator its parent 
					parent->UpdateKey(pIndex, path, depth - 1);
					return NULL;
				}

				//Attempt to merge with right sibling
				if (rightSib != NULL && (rightSib->_count + 1 + this->_count + 1) <= _tree._listCapacity)
				{
					//Grab all of rights items
					memcpy(&_keys[(_count + 1) *  _tree._keyType.Size], &rightSib->_keys[0], rightSib->_count *  _tree._keyType.Size);
					memcpy(&_values[(_count + 1) * sizeof(Node*)], &rightSib->_values[0], (rightSib->_count + 1) * sizeof(Node*));

					//Create new separator
					memcpy(&_keys[(_count) *  _tree._keyType.Size], (*(Node**)&rightSib->_values[0])->GetChildKey(),  _tree._keyType.Size);
					_count += (rightSib->_count + 1); //Count is the number of separators. + 1 is for the the one we are taking from parent.
					
					rightSib->_count = -1;
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
				if (leftSib != NULL && (leftSib->_count + 1 + this->_count + 1) <= _tree._listCapacity)
				{
					//Move everything over to left.
					memcpy(&leftSib->_keys[(leftSib->_count + 1) *  _tree._keyType.Size], &_keys[0], _count *  _tree._keyType.Size);
					memcpy(&leftSib->_values[(leftSib->_count + 1) * sizeof(Node*)], &_values[0], _count * sizeof(Node*));					
					
					//create new separator
					memcpy(&leftSib->_keys[(leftSib->_count) *  _tree._keyType.Size], (*(Node**)&_values[0])->GetChildKey(), _tree._keyType.Size);
					leftSib->_count += (_count + 1); //Count is the number of separators. + 1 is for the the one we are taking from parent.

					//Suicide
					_count = -1;
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
			memcpy(&_keys[index *  _tree._keyType.Size], &_keys[(index + 1) *  _tree._keyType.Size],  size *  _tree._keyType.Size);
			memcpy(&_values[(index + 1) * sizeof(Node*)], &_values[(index + 2) * sizeof(Node*)],  size * sizeof(Node*));

			_count--;
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
			return iterator(_values, _tree._valueType.Size);
		}

		iterator valueEnd()
		{
			return iterator(&_values[_count * _tree._valueType.Size], _tree._valueType.Size);
		}

	private:
		short _count;
		char* _keys;
		char* _values;
		BTree& _tree;
		bool _branch;

		short IndexOf(void* key)
		{
			auto result = _tree._keyType.IndexOf(_keys, _count, key);
			return result >= 0 ? result + 1 : ~result;
		}

		void InternalInsertIndex(short index, void* key, void* value)
		{
			int size = _count - index;
			if (_count != index)
			{
				memmove(&_keys[(index + 1) *  _tree._keyType.Size], &_keys[index  *  _tree._keyType.Size], size *  _tree._keyType.Size);
				if (!_tree._keyOnly)
					memmove(&_values[(index + 1) * _tree._valueType.Size], &_values[index * _tree._valueType.Size], size * _tree._valueType.Size);
			}

			 _tree._keyType.CopyIn(key, &_keys[index * _tree._keyType.Size]);
			if (!_tree._keyOnly)
				_tree._valueType.CopyIn(value, &_values[(index) * _tree._valueType.Size]);

			_count++;
		}

		void InternalInsertIndex(short index, void* key, Node* child)
		{
			int size = _count - index;
			if (_count != index)
			{
				memmove(&_keys[(index + 1) *  _tree._keyType.Size], &_keys[index  *  _tree._keyType.Size], size *  _tree._keyType.Size);
				memmove(&_values[(index +  2) * sizeof(Node*)], &_values[(index + 1) * sizeof(Node*)], size * sizeof(Node*));
			}

			 _tree._keyType.CopyIn(key, &_keys[index * _tree._keyType.Size]);
			standardtypes::StandardNodeType.CopyIn(&child, &_values[(index + 1) * sizeof(Node*)]);

			_count++;
		}
	

	friend class BTree::iterator;
	friend class BTree;
};

#pragma GCC diagnostic pop


