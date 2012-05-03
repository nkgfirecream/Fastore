#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Topology.h"
#pragma managed(pop)

#include "Utilities.h"

namespace Wrapper
{
	public ref class ManagedColumnDef
	{
		private:
			ColumnDef* _nativeColumnDef;

		public:
			ManagedColumnDef()
			{
				_nativeColumnDef = new ColumnDef();
			}

			ManagedColumnDef(ColumnDef* nativeColumnDef) : _nativeColumnDef(nativeColumnDef) {};
			ColumnDef* GetNativePointer();

			property int ColumnID
			{
				int get()
				{
					return _nativeColumnDef->ColumnID;
				}

				void set(int value)
				{
					_nativeColumnDef->ColumnID = value;
				}
			}


			property bool IsUnique
			{
				bool get()
				{
					return _nativeColumnDef->IsUnique;
				}

				void set(bool value)
				{
					_nativeColumnDef->IsUnique = value;
				}
			}

			property bool IsRequired
			{
				bool get()
				{
					return _nativeColumnDef->IsRequired;
				}

				void set(bool value)
				{
					_nativeColumnDef->IsRequired = value;
				}
			}

			property System::String^ Name
			{
				System::String^ get()
				{
					return Utilities::ConvertToManagedString(_nativeColumnDef->Name);
				}

				void set(System::String^ value)
				{
					_nativeColumnDef->Name = Utilities::ConvertToNativeWString(value);
				}
			}

			property System::String^ ValueType
			{
				System::String^ get()
				{
					return Utilities::ConvertScalarTypeToString(_nativeColumnDef->ValueType);
				}

				void set(System::String^ value)
				{
					_nativeColumnDef->ValueType = Utilities::ConvertStringToScalarType(value);
				}
			}

			property System::String^ RowIDType
			{
				System::String^ get()
				{
					return Utilities::ConvertScalarTypeToString(_nativeColumnDef->RowIDType);
				}

				void set(System::String^ value)
				{
					_nativeColumnDef->RowIDType = Utilities::ConvertStringToScalarType(value);
				}
			}
	};
}