#include "ColumnDef.h"

using namespace fastore::client;

const int &ColumnDef::getColumnID() const
{
	return privateColumnID;
}

void ColumnDef::setColumnID(const int &value)
{
	privateColumnID = value;
}

const std::string &ColumnDef::getName() const
{
	return privateName;
}

void ColumnDef::setName(const std::string &value)
{
	privateName = value;
}

const std::string &ColumnDef::getType() const
{
	return privateType;
}

void ColumnDef::setType(const std::string &value)
{
	privateType = value;
}

const std::string &ColumnDef::getIDType() const
{
	return privateIDType;
}

void ColumnDef::setIDType(const std::string &value)
{
	privateIDType = value;
}

const BufferType &ColumnDef::getBufferType() const
{
	return privateBufferType;
}

void ColumnDef::setBufferType(const BufferType &value)
{
	privateBufferType = value;
}
