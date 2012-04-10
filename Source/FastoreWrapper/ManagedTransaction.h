#pragma once
#include "stdafx.h"
#include "../FastoreCore/Transaction.h"
#include "ManagedDataSet.h"
#include "ManagedRange.h"

using namespace System;

namespace Wrapper
{
		//Wrappers for classes
	public ref class ManagedTransaction
	{
		private:
			Transaction* _nativeTransaction;

		public:
			Transaction* GetNativePointer();

			ManagedTransaction(Transaction* nativeTrans) : _nativeTransaction(nativeTrans) {};
			//void Dispose();
			void Commit();
			ManagedDataSet^ GetRange(array<System::Int32>^ columns, ManagedRange^ range/* [sorting]*/);
			ManagedDataSet^ GetRows(array<Object^>^ rowIds, array<System::Int32>^ columns/* Sorting*/);
			System::Object^ Include(array<Object^>^ row, array<System::Int32>^ columns, System::Boolean isPicky);
			void Exclude(array<Object^>^ rowsIds, array<System::Int32>^ columns, System::Boolean isPicky);
	};
}