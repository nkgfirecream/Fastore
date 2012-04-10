#include "stdafx.h"
#include "ManagedDatabase.h"

using namespace Wrapper;

Wrapper::ManagedSession^ Wrapper::ManagedDatabase::Start()
{
	//copy object to heap
	Session* session = new Session(_nativeDatabase->Start());

	//wrap object
	ManagedSession^ wrapper = gcnew ManagedSession(session);
	return wrapper;
}

TransactionID Wrapper::ManagedDatabase::GetID()
{
	TransactionID tid;
	return tid;
}

Database* Wrapper::ManagedDatabase::GetNativePointer()
{
	return _nativeDatabase;
}