#pragma once

#include <iterator>
#include "../Schema/tuple.h"

class DataSet
{
	char* _buffer;
	TupleType Type;
	int RowCount;
	public:

		DataSet(const TupleType tupleType, int rowCount) : Type(tupleType), RowCount(rowCount) 
		{
			_buffer = new char[Type.BufferSize * RowCount];
		}

		~DataSet()
		{
			delete[] _buffer;
		}

		void* operator[](int row)
		{
			return _buffer + (row * Type.BufferSize);
		}
		
		int ColumnOffset(int column)
		{
			int result = 0;
			for (int i = 0; i < column; i++)
				result += Type[i].Type.Size;
			return result;
		}

		void* Cell(int row, int column)
		{
			return (char*)operator[](row) + ColumnOffset(column);
		}

		class byColumn : public eastl::iterator<std::forward_iterator_tag, void*>
		{
				char* _buffer;
				size_t _rowsize;
				byColumn(char* buffer, size_t rowsize) : _buffer(buffer), _rowsize(rowsize) {}
			public:
				byColumn(const byColumn& iter) : _buffer(iter._buffer), _rowsize(iter._rowsize) {}
				byColumn& operator++() 
				{
					_buffer += _rowsize; 
					return *this;
				}
				byColumn operator++(int)
				{
					byColumn tmp(*this); 
					operator++(); 
					return tmp;
				}
				bool operator==(const byColumn& rhs) {return _buffer==rhs._buffer;}
				bool operator!=(const byColumn& rhs) {return _buffer!=rhs._buffer;}
				void* operator*() { return _buffer;}
			
			friend class DataSet;
		};

		byColumn beginColumn(int column)
		{
			return byColumn(_buffer + ColumnOffset(column), Type.BufferSize);
		}

		byColumn endColumn(int column)
		{
			return byColumn((char*)Cell(RowCount, column), Type.BufferSize);
		}

		class byRow : public eastl::iterator<std::forward_iterator_tag, void*>
		{
				char* _buffer;
				DataSet& _set;
				byRow(char* buffer, DataSet& set) : _buffer(buffer), _set(set) {}
			public:
				byRow(const byRow& iter) : _buffer(iter._buffer), _set(iter._set) {}
				byRow& operator++() 
				{
					_buffer += _set.Type.BufferSize; 
					return *this;
				}
				byRow operator++(int)
				{
					byRow tmp(*this); 
					operator++(); 
					return tmp;
				}
				bool operator==(const byRow& rhs) {return _buffer==rhs._buffer;}
				bool operator!=(const byRow& rhs) {return _buffer!=rhs._buffer;}
				void* operator*() { return _buffer;}
				void* Cell(int column)
				{
					return _buffer + _set.ColumnOffset(column);
				};
			
			friend class DataSet;
		};

		byRow beginRow()
		{
			return byRow(_buffer, this);
		}

		byRow endRow()
		{
			return byRow(_buffer, this);
		}
};