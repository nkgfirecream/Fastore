#include "ColumnHash.h"
#include <hash_set>

ColumnHash::ColumnHash(int(*compare)(void* left, void* right), wstring(*tostring)(void*))
{
	_compare = compare;

	_values = new BTree(128,128, _compare, tostring);

	_values->Observer = (IObserver*)this;
	_rows = new hash_map<long, Leaf*>();
}

void* ColumnHash::GetValue(long rowID)
{
	hash_map <long, Leaf*> :: const_iterator hash_mapIterator;
	hash_mapIterator = _rows->find(rowID);
	
	if(hash_mapIterator != _rows->end())
	{
		Leaf* leaf = hash_mapIterator->second;
		return leaf->GetKey(
			[rowID](void* hash) -> bool
			{
				hash_set<long>* newrows = (hash_set<long>*)hash;
				return newrows->find(rowID) != newrows->end();
			});
	}
	else
	{
		return NULL;
	}
}

typedef pair <long,Leaf*> LongLeafPair;
bool ColumnHash::Insert(long rowID, void* value)
{
	Leaf* valueLeaf;
	hash_set<long>* newrows = new hash_set<long>();
	void* existing = _values->Insert(value, newrows, valueLeaf);
	if(existing != NULL)
		newrows = (hash_set<long>*)existing;

	if(newrows->insert(rowID).second)
	{
		_rows->insert(LongLeafPair (rowID, valueLeaf));
		return true;
	}
	else
		return false;

}

void ColumnHash::ValuesMoved(void* lp)
{
	Leaf* leaf = (Leaf *)lp;
	for(int i = 0; i < leaf->Count; i++)
	{
		hash_set<long>* current = (hash_set<long>*)leaf->_values[i];
		hash_set<long> :: const_iterator hash_setIterator;
		hash_setIterator = current->begin();
		while(hash_setIterator != current->end())
		{
			_rows->find(*hash_setIterator)->second = leaf;
			hash_setIterator++;
		}
	}
}