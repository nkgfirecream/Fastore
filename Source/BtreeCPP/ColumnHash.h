#pragma once
#include "BTree.h"
#include "BTreeObserver.h"
#include <hash_map>

class ColumnHash : public IObserver
{
	public:
		ColumnHash(int(*)(void*,void*),wstring(*)(void*));
		void* GetValue(long);
		bool Insert(long, void*);
		//BatchInterface.. ChangeSet, execute all changes and return result;

	private:
		void IObserver::ValuesMoved(void *);
		int (*_compare)(void* left, void* right);
		hash_map<long, Leaf*>* _rows;
		BTree* _values;
};