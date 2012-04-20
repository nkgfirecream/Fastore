
#include "stdafx.h"
#include "ManagedSession.h"

using namespace Wrapper;

Session* Wrapper::ManagedSession::GetNativePointer()
{
	return _nativeSession;
}

Wrapper::ManagedSession::~ManagedSession()
{
	_nativeSession->Dispose();
}

void Wrapper::ManagedSession::Exclude(array<Object^>^ rowIds, array<System::String^>^ columns, System::Boolean isPicky)
{
	//Convert Ids.
	eastl::vector<void*> ids;
	
	//Run operation in native code.
	eastl::vector<fs::wstring> cols = Utilities::ConvertStringArray(columns);
	_nativeSession->Exclude(ids, cols, isPicky);

	//Pinned pointer should release bool when it goes out of scope here.
}

System::Object^  Wrapper::ManagedSession::Include(array<Object^>^ row, array<System::String^>^ columns, System::Boolean isPicky)
{
	//Convert row
	eastl::vector<void*> nativeRow;

	for (int i = 0; i < row->Length; i++)
	{
		auto type = row[i]->GetType();
		if (type == System::Int32::typeid)
		{
			int* intp = new int((int)row[i]);
			nativeRow.push_back(intp);
		}
		else if (type == System::Boolean::typeid)
		{
			bool* boolp = new bool((bool)row[i]);
			nativeRow.push_back(boolp);
		}
		else if (type == System::String::typeid)
		{
			std::wstring* pointer = new std::wstring(Utilities::ConvertString((System::String^)row[i]));
			nativeRow.push_back(pointer);
		}
		else
		{
			throw;
		}
	}	

	eastl::vector<fs::wstring> cols = Utilities::ConvertStringArray(columns);
	auto result = _nativeSession->Include(nativeRow, cols, isPicky);


	for (int i = 0; i < nativeRow.size(); i++)
	{
		delete nativeRow[i];
	}
	//Result will  be void*, which we then need to cast to some object. This means a topo lookup to determine what type of object it is... Or have rowId type predetermined somewhere;
	return gcnew System::Int32;
}

Wrapper::ManagedDataSet^ Wrapper::ManagedSession::GetRange(array<System::String^>^ columns, ManagedRange^ range, System::Int32 rangecolumn/* [sorting]*/)
{
	eastl::vector<fs::wstring> cols = Utilities::ConvertStringArray(columns);
	auto result = _nativeSession->GetRange(cols, *range->GetNativePointer(), rangecolumn);

	//Put a copy of the DataSet on the heap, since we are wrapping it -- TODO: This is another tradeoff between marshaling everything at once vs marshal on demand. Right now, I'm using Marshal on Demand
	DataSet* ds = new DataSet(result);

	return gcnew ManagedDataSet(ds);
}

Wrapper::ManagedDataSet^ Wrapper::ManagedSession::GetRows(array<Object^>^ rowIds, array<System::String^>^ columns/* Sorting*/)
{
	//ConvertIDs
	//Convert row
	eastl::vector<void*> nativeIds;

	for (int i = 0; i < rowIds->Length; i++)
	{
		auto type = rowIds[i]->GetType();
		if (type == System::Int32::typeid)
		{
			int* intp = new int((int)rowIds[i]);
			nativeIds.push_back(intp);
		}
		else if (type == System::Boolean::typeid)
		{
			bool* boolp = new bool((bool)rowIds[i]);
			nativeIds.push_back(boolp);
		}
		else if (type == System::String::typeid)
		{
			std::wstring* pointer = new std::wstring(Utilities::ConvertString((System::String^)rowIds[i]));
			nativeIds.push_back(pointer);
		}
		else
		{
			throw;
		}
	}	


	eastl::vector<fs::wstring> cols = Utilities::ConvertStringArray(columns);

	auto result = _nativeSession->GetRows(nativeIds, cols);

	//Put a copy of the DataSet on the heap, since we are wrapping it -- TODO: This is another tradeoff between marshaling everything at once vs marshal on demand. Right now, I'm using Marshal on Demand
	DataSet* ds = new DataSet(result);

	return gcnew ManagedDataSet(ds);
}

Wrapper::ManagedTransaction^ Wrapper::ManagedSession::Begin(System::Boolean readIsolation, System::Boolean writeIsolation)
{
	
	//copy object to heap
	Transaction* transaction = new Transaction(_nativeSession->Begin(readIsolation, writeIsolation));

	//wrap object
	ManagedTransaction^ wrapper = gcnew ManagedTransaction(transaction);
	return wrapper;
}



