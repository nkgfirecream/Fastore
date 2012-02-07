#pragma once

#include <iterator>
#include "../Schema/tuple.h"

class DataSet
{
	char* _buffer;
	public:
		const TupleType Type;
		const int RowCount;

		DataSet(const TupleType tupleType, int rowCount) : Type(tupleType), RowCount(rowCount) 
		{
			_buffer = new char[tupleType.BufferSize * RowCount];
		}

		~DataSet()
		{
			delete[] _buffer;
		}

		class iterator : public eastl::iterator<std::forward_iterator_tag, void*>
		{
				char* _buffer;
				size_t _rowsize;
				iterator(char* buffer, size_t rowsize) : _buffer(buffer), _rowsize(rowsize) {}
			public:
				iterator(const iterator& iter) : _buffer(iter._buffer), _rowsize(iter._rowsize) {}
				iterator& operator++() 
				{
					_buffer += _rowsize; 
					return *this;
				}
				iterator operator++(int)
				{
					iterator tmp(*this); 
					operator++(); 
					return tmp;
				}
				bool operator==(const iterator& rhs) {return _buffer==rhs._buffer;}
				bool operator!=(const iterator& rhs) {return _buffer!=rhs._buffer;}
				char* operator*() { return _buffer;}
		};

		// TODO: mutable iterator by column

		// TODO: mutable iterator by row
};