#pragma once
#include <iostream>
#include "BTree.h"
using namespace std;

class Leaf;

class IObserver
{
	public:
		virtual ~IObserver() {}
		inline virtual void ValuesMoved(Leaf* leaf);
};