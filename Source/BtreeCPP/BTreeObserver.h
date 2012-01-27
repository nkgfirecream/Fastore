#pragma once
#include<iostream>
using namespace std;

class IObserver
{
	public:
		virtual void ValuesMoved(void *) = 0;
};