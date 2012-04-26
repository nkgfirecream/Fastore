
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
	eastl::vector<void*> nativeRow(row->Length);

	for (int i = 0; i < row->Length; i++)
	{
		auto type = row[i]->GetType();
		if (type == System::Int32::typeid)
		{
			nativeRow[i] = new int((int)row[i]);
		}
		else if (type == System::Boolean::typeid)
		{
			nativeRow[i] = new bool((bool)row[i]);
		}
		else if (type == System::String::typeid)
		{
			nativeRow[i] = new std::wstring(Utilities::ConvertString((System::String^)row[i]));
		}
		else
		{
			throw;
		}
	}	

	eastl::vector<fs::wstring> cols = Utilities::ConvertStringArray(columns);
	auto result = _nativeSession->Include(nativeRow, cols, isPicky);


	for (unsigned int i = 0; i < nativeRow.size(); i++)
	{
		delete nativeRow[i];
	}
	//Result will  be void*, which we then need to cast to some object. This means a topo lookup to determine what type of object it is... Or have rowId type predetermined somewhere;
	return gcnew System::Int32;
}

Wrapper::ManagedDataSet^ Wrapper::ManagedSession::GetRange(array<System::String^>^ columns, array<ManagedRange^>^ ranges)
{
	eastl::vector<fs::wstring> cols = Utilities::ConvertStringArray(columns);

	eastl::vector<Range> rgs;
	for (int i = 0; i < ranges->Length; i++)
	{
		rgs.push_back((*ranges[i]->GetNativePointer()));
	}

	auto result = _nativeSession->GetRange(cols, rgs);

	//Put a copy of the DataSet on the heap, since we are wrapping it -- TODO: This is another tradeoff between marshaling everything at once vs marshal on demand. Right now, I'm using Marshal on Demand
	DataSet* ds = new DataSet(result);

	return gcnew ManagedDataSet(ds);
}

Wrapper::ManagedDataSet^ Wrapper::ManagedSession::GetRows(array<Object^>^ rowIds, array<System::String^>^ columns/* Sorting*/)
{
	//ConvertIDs
	//Convert row
	eastl::vector<void*> nativeIds(rowIds->Length);

	for (int i = 0; i < rowIds->Length; i++)
	{
		auto type = rowIds[i]->GetType();
		if (type == System::Int32::typeid)
		{
			nativeIds[i] = new int((int)rowIds[i]);
		}
		else if (type == System::Boolean::typeid)
		{
			nativeIds[i] = new bool((bool)rowIds[i]);
		}
		else if (type == System::String::typeid)
		{
			nativeIds[i] = new std::wstring(Utilities::ConvertString((System::String^)rowIds[i]));
		}
		else
		{
			throw;
		}
	}	

	eastl::vector<fs::wstring> cols = Utilities::ConvertStringArray(columns);

	auto result = _nativeSession->GetRows(nativeIds, cols);

	for (unsigned int i = 0; i < nativeIds.size(); i++)
	{
		delete nativeIds[i];
	}

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



