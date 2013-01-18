#include "BufferFactory.h"

std::unique_ptr<IColumnBuffer> BufferFactory::CreateBuffer(const std::string& valueTypeName, const std::string& rowIdTypeName, const BufferType_t& bufferType)
{
	const ScalarType& valueType(standardtypes::GetTypeFromName(valueTypeName));
	const ScalarType& rowType(standardtypes::GetTypeFromName(rowIdTypeName));

	if (bufferType == BufferType_t::Identity)
	{
		//TODO: write equality and inequality operators for scalartypes
		//This might have some effects where they are used in hash tables
		//etc., so I haven't looked at that yet.
		if (&valueType != &rowType)
			throw "Identity Buffers require rowType and ValueType to be the same";

		return std::unique_ptr<IColumnBuffer>(new IdentityBuffer(valueType));
	}
	else if(bufferType == BufferType_t::Unique)
	{
		//8 is the size of a pointer. If the size is less than 8, it's cheaper (from a memory point of view) to duplicate the value in the reverse index than it is to track pointers and update.
		if (valueType.Size <= 8) 
		{
			return std::unique_ptr<IColumnBuffer>(new UniqueInlineBuffer(rowType, valueType));
		}
		else
		{
			return std::unique_ptr<IColumnBuffer>(new UniqueBuffer(rowType, valueType));
		}
	}
	else
	{
		//if (_def.ValueType.Size <= 8)
		//{
		//	_buffer = std::unique_ptr<IColumnBuffer>(new TreeInlineBuffer(_def.RowIDType, _def.ValueType));		
		//}
		//else
		//{
			return std::unique_ptr<IColumnBuffer>(new TreeBuffer(rowType, valueType));		
		//}
	}
}
