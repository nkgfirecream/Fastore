#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Session.h"
#pragma managed(pop)

#include "ManagedDataSet.h"
#include "ManagedTransaction.h"

using namespace System;

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
			ManagedDataSet^ GetRange(array<System::Int32>^ columns, ManagedRange^ range/* [sorting]*/);
			ManagedDataSet^ GetRows(array<Object^>^ rowIds, array<System::Int32>^ columns/* Sorting*/);
			System::Object^ Include(array<Object^>^ row, array<System::Int32>^ columns, System::Boolean isPicky);
			void Exclude(array<Object^>^ rowsIds, array<System::Int32>^ columns, System::Boolean isPicky);
	};
}