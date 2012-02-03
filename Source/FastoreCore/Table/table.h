#pragma once

#include <memory>
#include "../Schema/tuple.h"
#include "../Column/columnbuffer.h"
#include "../ColumnHash.h"

class Table
{
		TupleType _type; 
		ScalarType _rowType;
		eastl::vector<std::unique_ptr<ColumnBuffer>> _buffers;	
	public:
		Table(const TupleType& type) : _type(type), _rowType(GetLongType()) 
		{
			for (TupleType::iterator it = _type.begin(); it != _type.end(); ++it)
			{
				_buffers.push_back(std::unique_ptr<ColumnBuffer>(new ColumnHash(_rowType, (*it).Type))); 
			}
		}

		const TupleType& getType() { return _type; };
}