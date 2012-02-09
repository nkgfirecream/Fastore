#pragma once

#include <EASTL/vector.h>
#include "column.h"
#include "../typedefs.h"
#include <sstream>
#include <exception>

using namespace std;

typedef eastl::vector<const ColumnType> ColumnTypeVector;

class TupleType
{
		const ColumnTypeVector _columns;

		size_t bufferSize()
		{
			size_t result = 0;
			for (ColumnTypeVector::iterator it = _columns.begin(); it != _columns.end(); ++it)
				result += (*it).Type.Size;
		}

	public:
		const size_t BufferSize;

		TupleType(ColumnTypeVector columns) : _columns(columns), BufferSize(bufferSize()) {};

		const ColumnType& operator[] (int index)
		{
			return _columns[index];
		}

		const ColumnType& operator[] (fs::wstring name)
		{
			for (ColumnTypeVector::iterator it = _columns.begin(); it != _columns.end(); ++it)
				if ((*it).Name.compare(name) == 0)
					return *it;
			stringstream error;
			error << "Column name '" << string(name.begin(), name.end()) << "' not found.";
			throw std::exception(error.str().c_str());
		}

		typedef ColumnTypeVector::const_iterator iterator;

		iterator begin() const
		{
			return _columns.begin();
		}

		iterator end() const
		{
			return _columns.end();
		}

		int size() const
		{
			return _columns.size();
		}
};