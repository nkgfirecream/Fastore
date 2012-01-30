//#pragma once
//#include "BTree.h"
//#include "BTreeObserver.h"
//#include <hash_map>
//
//template<class K, class V>
//class ColumnHash : public IObserver<K,V>
//{
//	public:
//		ColumnHash(int(*)(K,K),wstring(*)(K), wstring(*)(V));
//		K GetValue(V);
//		bool Insert(K, V);
//
//	private:
//		void ValuesMoved(Leaf<K,V>*);
//		int (*_compare)(K left, K right);
//		hash_map<V, Leaf<K,V>*>* _rows;
//		BTree<K,V>* _values;
//};
//
//
//template <class K, class V>
//inline ColumnHash<K,V>::ColumnHash(int(*compare)(K, K), wstring(*keyToString)(K), wstring(*valueToString)(V))
//{
//	_compare = compare;
//
//	_values = new BTree<K,V>(128,128, _compare, keyToString, valueToString);
//
//	_values->Observer = (IObserver<K,V>*)this;
//	_rows = new hash_map<V, Leaf<K,V>*>();
//}
//
//template <class K, class V>
//inline K ColumnHash<K,V>::GetValue(V rowId)
//{
//	hash_map <V, Leaf<K,V>*> :: const_iterator hash_mapIterator;
//	hash_mapIterator = _rows->find(rowId);
//	
//	if(hash_mapIterator != _rows->end())
//	{
//		Leaf<K,V>* leaf = hash_mapIterator->second;
//		return leaf->GetKey(
//			[rowId](V hash) -> bool
//			{
//				hash_set<V>* newrows = (hash_set<V>*)hash;
//				return newrows->find(rowID) != newrows->end();
//			});
//	}
//	else
//	{
//		return NULL;
//	}
//}
//
//template <class K, class V>
//inline bool ColumnHash<K,V>::Insert(K value, V rowId)
//{
//	typedef pair <V,Leaf<K,V>*> ValueLeafPair;
//	Leaf<K,V>* valueLeaf;
//	hash_set<V>* newrows = new hash_set<V>();
//	void* existing = _values->Insert(value, newrows, valueLeaf);
//	if(existing != NULL)
//		newrows = (hash_set<V>*)existing;
//
//	if(newrows->insert(rowId).second)
//	{
//		_rows->insert(ValueLeafPair (rowId, valueLeaf));
//		return true;
//	}
//	else
//		return false;
//
//}
//
//template <class K, class V>
//inline void ColumnHash<K,V>::ValuesMoved(Leaf<K,V>* leaf)
//{
//	for(int i = 0; i < leaf->Count; i++)
//	{
//		hash_set<V>* current = (hash_set<V>*)leaf->_values[i];
//		hash_set<V> :: const_iterator hash_setIterator;
//		hash_setIterator = current->begin();
//		while(hash_setIterator != current->end())
//		{
//			_rows->find(*hash_setIterator)->second = leaf;
//			hash_setIterator++;
//		}
//	}
//}