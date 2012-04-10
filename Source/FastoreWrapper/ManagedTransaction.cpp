
#include "stdafx.h"
#include "ManagedTransaction.h"

using namespace Wrapper;

Transaction* Wrapper::ManagedTransaction::GetNativePointer()
{
	return _nativeTransaction;
}

Wrapper::ManagedTransaction::~ManagedTransaction()
{
	_nativeTransaction->Dispose();
}

void Wrapper::ManagedTransaction::Commit()
{
	_nativeTransaction->Commit();
}

void Wrapper::ManagedTransaction::Exclude(array<Object^>^ rowIds, array<System::Int32>^ columns, System::Boolean isPicky)
{
	int size = rowIds->Length;

	//Can't put dynamically sized arrays on the stack, so allocate them on the heap
	void** ids = new void*[size];
	for (int i = 0; i < size; i++)
	{
		//TODO: Object marshaling and conversion code.
		//Strings need to be copied, ints, longs, etc. pinned.
		//ids[i] = ConvertObject(rowIds[i]);
	}

	//Convert columnIds to int[] --- TODO: Consider using a pinned pointer to the first element and the adapt the api to use a pointer (this doesn't figure size of array)
	int* cols = new int[size];
	for (int i = 0; i < size; i++)
	{
		cols[i] = columns[i];
	}

	//Run operation in native code.
	_nativeTransaction->Exclude(ids, cols, isPicky);

	//TODO: Do we new to convert back to the correctly typed pointer before deleteing?
	delete[] ids;
	delete[] cols;

	//Pinned pointer should release bool when it goes out of scope here.
}

System::Object^  Wrapper::ManagedTransaction::Include(array<Object^>^ row, array<System::Int32>^ columns, System::Boolean isPicky)
{
	int size = row->Length;
	//Can't put dynamically sized arrays on the stack, so allocate them on the heap

	void** nativeRow = new void*[size];
	for (int i = 0; i < size; i++)
	{
		//TODO: Object marshaling and conversion code.
		//Strings need to be copied, ints, longs, etc. pinned.
		//ids[i] = ConvertObject(rowIds[i]);
	}

	//Convert columnIds to int[] --- TODO: Consider using a pinned pointer to the first element and the adapt the api to use a pointer (this doesn't figure size of array)
	int* cols = new int[size];
	for (int i = 0; i < size; i++)
	{
		cols[i] = columns[i];
	}

	//TODO: fix return type
	auto result = _nativeTransaction->Include(nativeRow, cols, isPicky);

	//Result will  be void*, which we then need to cast to some object. This means a topo lookup to determine what type of object it is... Or have rowId type predetermined somewhere;
	return gcnew System::Int32;
}

Wrapper::ManagedDataSet^ Wrapper::ManagedTransaction::GetRange(array<System::Int32>^ columns, ManagedRange^ range/* [sorting]*/)
{
	int size = columns->Length;
	int* cols = new int[size];

	for (int i = 0; i < size; i++)
	{
		cols[i] = columns[i];
	}

	auto result = _nativeTransaction->GetRange(cols, *range->GetNativePointer());

	//Put a copy of the DataSet on the heap, since we are wrapping it -- TODO: This is another tradeoff between marshaling everything at once vs marshal on demand. Right now, I'm using Marshal on Demand
	DataSet* ds = new DataSet(result);

	return gcnew ManagedDataSet(ds);
}

Wrapper::ManagedDataSet^ Wrapper::ManagedTransaction::GetRows(array<Object^>^ rowIds, array<System::Int32>^ columns/* Sorting*/)
{
	//Can't put dynamically sized arrays on the stack, so allocate them on the heap

	int size = rowIds->Length;
	void** ids = new void*[rowIds->Length];
	for (int i = 0; i < size; i++)
	{
		//TODO: Object marshaling and conversion code.
		//Strings need to be copied, ints, longs, etc. pinned.
		//ids[i] = ConvertObject(rowIds[i]);
	}

	//Convert columnIds to int[] --- TODO: Consider using a pinned pointer to the first element and the adapt the api to use a pointer (this doesn't figure size of array)
	size = columns->Length;
	int* cols = new int[size];

	for (int i = 0; i < size; i++)
	{
		cols[i] = columns[i];
	}

	auto result = _nativeTransaction->GetRows(ids,cols);

	//Put a copy of the DataSet on the heap, since we are wrapping it -- TODO: This is another tradeoff between marshaling everything at once vs marshal on demand. Right now, I'm using Marshal on Demand
	DataSet* ds = new DataSet(result);

	return gcnew ManagedDataSet(ds);
}
