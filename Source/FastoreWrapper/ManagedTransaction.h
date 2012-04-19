#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Transaction.h"
#pragma managed(pop)

#include "ManagedDataSet.h"
#include "ManagedRange.h"

namespace Wrapper
{
		//Wrappers for classes
	public ref class ManagedTransaction
	{
		private:
			Transaction* _nativeTransaction;

		public:
			Transaction* GetNativePointer();

			~ManagedTransaction();
			//!ManagedTransaction();
			ManagedTransaction(Transaction* nativeTrans) : _nativeTransaction(nativeTrans) {};

			void Commit();
			ManagedDataSet^ GetRange(array<System::Int32>^ columns, ManagedRange^ range/* [sorting]*/);
			ManagedDataSet^ GetRows(array<System::Object^>^ rowIds, array<System::Int32>^ columns/* Sorting*/);
			System::Object^ Include(array<System::Object^>^ row, array<System::Int32>^ columns, System::Boolean isPicky);
			void Exclude(array<System::Object^>^ rowsIds, array<System::Int32>^ columns, System::Boolean isPicky);
	};
}