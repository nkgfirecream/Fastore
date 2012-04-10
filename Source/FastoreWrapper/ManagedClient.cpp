#include "stdafx.h"
#include "ManagedClient.h"

using namespace Wrapper;

Wrapper::ManagedDatabase^ Wrapper::ManagedClient::Connect(ManagedHost^ host)
{
	//create database based on host
	Database* database = new Database();

	ManagedDatabase^ wrapper = gcnew ManagedDatabase(database);
	return wrapper;
}

Client* Wrapper::ManagedClient::GetNativePointer()
{
	return _nativeClient;
}