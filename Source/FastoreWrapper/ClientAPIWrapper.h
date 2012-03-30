#pragma once
#include "stdafx.h"
#include "../FastoreCore/ClientAPI.h"

namespace Wrapper
{
	public ref class ITransactionWrapper
	{
		private:
			ITransaction* _nativeTransaction;

		public:
			//void Dispose();
			void Commit();
			DataSet GetRange(/*Columns, [range], [sorting]*/);
			DataSet GetRows(/* RowIds[], Columns, Sorting*/);
			void* Include(/* row, columns, isPicky*/);
			void Exclude (/* rowid[], columns, ispicky */);
	};

	public ref class ISessionWrapper
	{
		private:
			ISession* _nativeSession;

		public:
			//void Dispose();
			ITransactionWrapper^ Begin(bool ReadIsolation, bool WriteIsolation);
			DataSet GetRange(/*Columns, [range], [sorting]*/);
			DataSet GetRows(/* RowIds[], Columns, Sorting*/);
			void* Include(/* row, columns, isPicky*/);
			void Exclude (/* rowid[], columns, ispicky */);
	};

	public ref class IDatabaseWrapper
	{
		private:
			IDatabase* _nativeDatabase;

		public:
			ISessionWrapper^ Start();
			TransactionID GetID(/* Token */);
			//Low priority
			//void Topology();
			//ILock GetLock(/*name, mode */);
	};

	public ref class IHostWrapper
	{
		private:
			IHost* _nativeHost;
	};

	public ref class HostFactoryWrapper
	{
		private:
			HostFactory* _nativeHostFactory;

		public:
			IHostWrapper^ Create(Topology topo);
			//Lower Priority
			//IHost Connect(address);
		
	};

	public ref class ClientWrapper
	{
		private: 
			Client* _nativeClient;

		public:
			IDatabaseWrapper^ Connect(IHostWrapper^ host);
	};
}