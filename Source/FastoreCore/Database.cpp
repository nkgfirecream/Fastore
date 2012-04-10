#include "Database.h"
#include "TransactionID.h"
#include "Session.h"

TransactionID Database::GetID()
{
	return TransactionID();
}

Session Database::Start()
{
	return Session();
}