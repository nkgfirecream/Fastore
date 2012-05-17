#include "Fastore_types.h"

//This need to be defined for ordering sets/lists since Thrift depends on sets/lists

bool fastore::Host::operator<(const fastore::Host& other) const
{
	return ID < other.ID;
}

bool fastore::Repository::operator<(const fastore::Repository& other) const
{
	return hostID < other.hostID && columnID < other.columnID;
}

bool fastore::Include::operator<(const fastore::Include& other) const
{
	return Value < other.Value && RowID < other.RowID;
}

bool fastore::Exclude::operator<(const fastore::Exclude& other) const
{
	return RowID < other.RowID;
}

bool fastore::Query::operator<(const fastore::Query& other) const
{
	return true;
}

