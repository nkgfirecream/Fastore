
#include "stdafx.h"
#include "ClientAPIWrapper.h"

using namespace Wrapper;

void Wrapper::ITransactionWrapper::Commit()
{
	_nativeTransaction->Commit();
}

void Wrapper::ITransactionWrapper::Exclude()
{
	_nativeTransaction->Exclude();
}

void* Wrapper::ITransactionWrapper::Include()
{
	return _nativeTransaction->Include();
}

DataSet Wrapper::ITransactionWrapper::GetRange()
{
	return _nativeTransaction->GetRange();
}

DataSet Wrapper::ITransactionWrapper::GetRows()
{
	return _nativeTransaction->GetRows();
}

void Wrapper::ISessionWrapper::Exclude()
{
	_nativeSession->Exclude();
}

void* Wrapper::ISessionWrapper::Include()
{
	return _nativeSession->Include();
}

DataSet Wrapper::ISessionWrapper::GetRange()
{
	return _nativeSession->GetRange();
}

DataSet Wrapper::ISessionWrapper::GetRows()
{
	return _nativeSession->GetRows();
}

Wrapper::ITransactionWrapper^ Wrapper::ISessionWrapper::Begin(bool ReadIsolation, bool WriteIsolation)
{
	ITransactionWrapper^ wrapper = gcnew ITransactionWrapper;
	return wrapper;
}

Wrapper::ISessionWrapper^ Wrapper::IDatabaseWrapper::Start()
{
	ISessionWrapper^ wrapper = gcnew ISessionWrapper;
	return wrapper;
}

TransactionID Wrapper::IDatabaseWrapper::GetID()
{
	TransactionID tid;
	return tid;
}

Wrapper::IHostWrapper^ Wrapper::HostFactoryWrapper::Create(Topology topo)
{
	IHostWrapper^ wrapper = gcnew IHostWrapper;
	return wrapper;
}

Wrapper::IDatabaseWrapper^ Wrapper::ClientWrapper::Connect(IHostWrapper^ host)
{
	IDatabaseWrapper^ wrapper = gcnew IDatabaseWrapper;
	return wrapper;
}


