#pragma once
#include "../Buffer/UniqueBuffer.h"
#include "../Buffer/UniqueInlineBuffer.h"
#include "../Buffer/TreeBuffer.h"
#include "../Buffer/TreeInlineBuffer.h"
#include "../Buffer/IdentityBuffer.h"
#include "../Type/Standardtypes.h"
#include "BufferType.h"

class BufferFactory
{
public:
	static std::unique_ptr<IColumnBuffer> CreateBuffer(const std::string& valueTypeName, const std::string& rowIdTypeName, const BufferType_t& bufferType);
};