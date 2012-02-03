#pragma once

#include "tuple.h"

class DataSet
{
	char* _buffer;
	public:
		const TupleType Type;

		DataSet(const TupleType tupleType) : Type(tupleType) 
		{
			_buffer = new char[tupleType.BufferSize];
		}

		~DataSet()
		{
			delete[] _buffer;
		}

		class iterator : public std::iterator<output_iterator_tag, void*>
		{
				char* _buffer;
				size_t _rowsize;
				iterator(char* buffer, size_t rowsize) : _buffer(buffer), _rowsize(rowsize) {}
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
			friend class Leaf;
		};

		// TODO: mutable iterator by column

		// TODO: mutable iterator by row
}