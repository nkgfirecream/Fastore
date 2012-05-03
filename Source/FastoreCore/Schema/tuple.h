#pragma once

#include <EASTL/vector.h>
#include "column.h"
#include "../typedefs.h"
#include <sstream>
#include <exception>

using namespace std;

typedef eastl::vector<ColumnDef> ColumnDefVector;

class TupleType
{
		ColumnDefVector _columns;

		size_t bufferSize()
		{
			size_t result = 0;
			for (ColumnDefVector::iterator it = _columns.begin(); it != _columns.end(); ++it)
				result += (*it).ValueType.Size;

			return result;
		}

	public:
		const size_t BufferSize;

		TupleType(ColumnDefVector columns) : _columns(columns), BufferSize(bufferSize()) {};

		const ColumnDef& operator[] (int index)
		{
			return _columns[index];
		}

		const ColumnDef& operator[] (fs::wstring name)
		{
			for (ColumnDefVector::iterator it = _columns.begin(); it != _columns.end(); ++it)
				if ((*it).Name.compare(name) == 0)
					return *it;
			wstringstream error;
			error << "Column name '" << fs::wstring(name.begin(), name.end()) << "' not found.";
			throw std::exception(fs::string(error.str().begin(), error.str().end()).c_str());
		}

		typedef ColumnDefVector::const_iterator iterator;

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