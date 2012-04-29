
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

void Wrapper::ManagedSession::Exclude(array<Object^>^ rowIds, array<System::String^>^ columns)
{
	//Convert Ids.
	eastl::vector<void*> ids;
	
	//Run operation in native code.
	eastl::vector<fs::wstring> cols = Utilities::ConvertStringArray(columns);
	_nativeSession->Exclude(ids, cols);

	//Pinned pointer should release bool when it goes out of scope here.
}

void  Wrapper::ManagedSession::Include(Object^ rowId, array<Object^>^ row, array<System::String^>^ columns)
{
	//Convert row
	eastl::vector<void*> nativeRow(row->Length);

	for (int i = 0; i < row->Length; i++)
	{
			nativeRow[i] = Utilities::ConvertObjectToNative(row[i]);
	}	

	eastl::vector<fs::wstring> cols = Utilities::ConvertStringArray(columns);

	void* rowIdp = Utilities::ConvertObjectToNative(rowId);

	_nativeSession->Include(rowIdp, nativeRow, cols);


	delete rowIdp;
	for (unsigned int i = 0; i < nativeRow.size(); i++)
	{
		delete nativeRow[i];
	}
}

Wrapper::ManagedDataSet^ Wrapper::ManagedSession::GetRange(array<System::String^>^ columns, array<ManagedOrder^>^ orders, array<ManagedRange^>^ ranges)
{
	eastl::vector<fs::wstring> cols = Utilities::ConvertStringArray(columns);

	eastl::vector<Range> rgs;
	for (int i = 0; i < ranges->Length; i++)
	{
		rgs.push_back((*ranges[i]->GetNativePointer()));
	}

	eastl::vector<Order> ords;
	for (int i = 0; i < orders->Length; i++)
	{
		ords.push_back((*orders[i]->GetNativePointer()));
	}

	auto result = _nativeSession->GetRange(cols, ords, rgs);

	//Put a copy of the DataSet on the heap, since we are wrapping it -- TODO: This is another tradeoff between marshaling everything at once vs marshal on demand. Right now, I'm using Marshal on Demand
	DataSet* ds = new DataSet(result);

	return gcnew ManagedDataSet(ds);
}

Wrapper::ManagedDataSet^ Wrapper::ManagedSession::GetRows(array<Object^>^ rowIds, array<System::String^>^ columns)
{
	//ConvertIDs
	//Convert row
	eastl::vector<void*> nativeIds(rowIds->Length);

	for (int i = 0; i < rowIds->Length; i++)
	{
		nativeIds[i] = Utilities::ConvertObjectToNative(rowIds[i]);
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

Wrapper::ManagedStatistics^ Wrapper::ManagedSession::GetStatistics(System::String^ column)
{
	fs::wstring col = Utilities::ConvertToNativeWString(column);
	Statistics* stats = new Statistics(_nativeSession->GetStatistics(col));
	ManagedStatistics^ mstats = gcnew ManagedStatistics(stats);
	return mstats;
}



