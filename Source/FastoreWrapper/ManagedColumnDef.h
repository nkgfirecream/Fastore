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

			property System::String^ Name
			{
				System::String^ get()
				{
					return Utilities::ConvertString(_nativeColumnDef->Name);
				}

				void set(System::String^ value)
				{
					_nativeColumnDef->Name = Utilities::ConvertString(value);
				}
			}

			property System::String^ KeyType
			{
				System::String^ get()
				{
					return Utilities::ConvertString(_nativeColumnDef->KeyType);
				}

				void set(System::String^ value)
				{
					_nativeColumnDef->KeyType = Utilities::ConvertString(value);
				}
			}
	};
}