#include "Comm_types.h"

//This need to be defined for ordering sets/lists since Thrift depends on sets/lists

bool fastore::communication::Include::operator<(const fastore::communication::Include& other) const
{
	return value.compare(other.value) < 0 && rowID.compare(other.rowID) < 0;
}

bool fastore::communication::Exclude::operator<(const fastore::communication::Exclude& other) const
{
	return rowID.compare(other.rowID) < 0;
}

bool fastore::communication::Query::operator<(const fastore::communication::Query& other) const
{
	return true;
}

bool fastore::communication::TransactionID::operator<(const fastore::communication::TransactionID& other) const
{
	return revision < other.revision;
}

bool fastore::communication::OptionalValue::operator<(const fastore::communication::OptionalValue& other) const
{
	if (__isset.value && other.__isset.value)
		return value.compare(other.value) < 0;
	else if(!__isset.value && other.__isset.value)
		return true;
	else
		return false;
}

