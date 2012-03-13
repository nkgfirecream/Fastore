#pragma once
#include "stdafx.h"
#include "../FastoreCore/FastoreAPI.h"

namespace Wrapper
{
	public ref class ITransactionWrapper
	{
		public:
			void Dispose();
			void Commit();
			DataSet GetRange(/*Columns, [range], [sorting]*/);
			DataSet GetRows(/* RowIds[], Columns, Sorting*/);
			void* Include(/* row, columns, isPicky*/);
			void Exclude (/* rowid[], columns, ispicky */);
	};

	public ref class ISessionWrapper
	{
		public:
			void Dispose();
			ITransaction Begin(bool ReadIsolation, bool WriteIsolation);
			DataSet GetRange(/*Columns, [range], [sorting]*/);
			DataSet GetRows(/* RowIds[], Columns, Sorting*/);
			void* Include(/* row, columns, isPicky*/);
			void Exclude (/* rowid[], columns, ispicky */);
	};

	public ref class IDatabaseWrapper
	{
		public:
			ISessionWrapper Start();
			TransactionID GetID(/* Token */);
			//Low priority
			//void Topology();
			//ILock GetLock(/*name, mode */);
	};

	public ref class HostFactoryWrapper
	{
		public:
			IHostWrapper Create(Topology topo);
			//Lower Priority
			//IHost Connect(address);
		
	};

	public ref class ClientWrapper
	{
		public:
			IDatabaseWrapper Connect(IHost host);
	};
}