#include "stdafx.h"
#include "ManagedDataSet.h"
#include "Utilities.h"

DataSet* Wrapper::ManagedDataSet::GetNativePointer()
{
	return _nativeDataSet;
}

void Wrapper::ManagedDataSet::Dump()
{
	for(int i = 0; i < _nativeDataSet->Size(); i++)
	{
		for(int j = 0; j < 6; j++)
		{
			System::Console::Write(Utilities::ConvertString((_nativeDataSet->Type)[j].Type.ToString(_nativeDataSet->Cell(i,j))));
			System::Console::Write(L" ");
		}
		System::Console::WriteLine();
	}
}

System::Int32 Wrapper::ManagedDataSet::Size()
{
	return _nativeDataSet->Size();
}

array<System::String^>^  Wrapper::ManagedDataSet::Row(System::Int32 rownum)
{
	int numcolumns = (_nativeDataSet->Type).size();
	array<System::String^>^ cells = gcnew array<System::String^>(numcolumns);
	for(int i = 0; i < numcolumns; i++)
	{
		cells[i] = Utilities::ConvertString((_nativeDataSet->Type)[i].Type.ToString(_nativeDataSet->Cell(rownum, i)));
	}

	return cells;
}

