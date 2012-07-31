#include "RangeSet.h"

using namespace fastore::client;

const bool &RangeSet::getEof() const
{
	return _Eof;
}

void RangeSet::setEof(const bool &value)
{
	_Eof = value;
}

const bool &RangeSet::getBof() const
{
	return _Bof;
}

void RangeSet::setBof(const bool &value)
{
	_Bof = value;
}

const bool &RangeSet::getLimited() const
{
	return _Limited;
}

void RangeSet::setLimited(const bool &value)
{
	_Limited = value;
}

const DataSet &RangeSet::getData() const
{
	return _Data;
}

void RangeSet::setData(const DataSet &value)
{
	_Data = value;
}