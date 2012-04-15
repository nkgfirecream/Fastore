#include "Session.h"
#include "Transaction.h"

//ISession
Transaction Session::Begin(bool readIsolation, bool writeIsolation)
{
	return Transaction(_host);
}

void Session::Dispose()
{

}