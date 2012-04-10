#pragma once
#include "stdafx.h"
#include "../FastoreCore/Session.h"
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

			ManagedSession(Session* nativeSession) : _nativeSession(nativeSession) {};
			//void Dispose();
			ManagedTransaction^ Begin(System::Boolean readIsolation, System::Boolean writeIsolation);
			ManagedDataSet^ GetRange(array<System::Int32>^ columns, ManagedRange^ range/* [sorting]*/);
			ManagedDataSet^ GetRows(array<Object^>^ rowIds, array<System::Int32>^ columns/* Sorting*/);
			System::Object^ Include(array<Object^>^ row, array<System::Int32>^ columns, System::Boolean isPicky);
			void Exclude(array<Object^>^ rowsIds, array<System::Int32>^ columns, System::Boolean isPicky);
	};
}