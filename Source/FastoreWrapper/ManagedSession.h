#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Session.h"
#pragma managed(pop)

#include "ManagedDataSet.h"
#include "ManagedTransaction.h"
#include "ManagedStatistics.h"

namespace Wrapper
{
	public ref class ManagedSession
	{
		private:
			Session* _nativeSession;

		public:
			Session* GetNativePointer();

			~ManagedSession(); // This is how IDisposable is implemented
			//!ManagedSession(); //This is how a finalizer is implemented
			ManagedSession(Session* nativeSession) : _nativeSession(nativeSession) {};


			ManagedTransaction^ Begin(System::Boolean readIsolation, System::Boolean writeIsolation);
			ManagedDataSet^ GetRange(array<System::Int32>^ columns, array<ManagedOrder^>^ orders, array<ManagedRange^>^ ranges);
			ManagedDataSet^ GetRows(array<System::Object^>^ rowIds, array<System::Int32>^ columns);
			void Include(Object^ rowId, array<System::Object^>^ row, array<System::Int32>^ columns);
			void Exclude(Object^ rowsId, array<System::Int32>^ columns);
			ManagedStatistics^ GetStatistics(System::Int32 columnId);
	};
}