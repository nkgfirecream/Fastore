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