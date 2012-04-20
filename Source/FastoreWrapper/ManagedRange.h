#pragma once
#include "stdafx.h"
#include "Utilities.h"

#pragma managed(push, off)
#include "../FastoreCore/Range.h"
#pragma managed(pop)

namespace Wrapper
{
	//TODO: These should probably be value types... 
	public ref class ManagedRangeBound
	{
		private:
			fs::RangeBound* _nativeRangeBound;
			void* valuep;
			void* idp;
			void* rowIdp;

		public:
			fs::RangeBound* GetNativePointer();

			ManagedRangeBound(System::Object^ value, System::Object^ rowId, System::Boolean inclusive)
			{
				//Convert object to void*
				
				//Refactor this code into utilities... It appears all over.
				auto type = value->GetType();
				if (type == System::Int32::typeid)
				{
					valuep = new int((int)value);
				}
				else if (type == System::Boolean::typeid)
				{
					valuep = new bool((bool)value);
				}
				else if (type == System::String::typeid)
				{
					valuep = new std::wstring(Utilities::ConvertString((System::String^)value));
				}
				else
				{
					throw;
				}

				Optional<void*>* rowIdp;

				if (rowId != nullptr)
				{
					auto type = rowId->GetType();
					if (type == System::Int32::typeid)
					{
						idp = new int((int)value);
					}
					else if (type == System::Boolean::typeid)
					{
						idp = new bool((bool)value);
					}
					else if (type == System::String::typeid)
					{
						idp = new std::wstring(Utilities::ConvertString((System::String^)value));
					}
					else
					{
						throw;
					}

					rowIdp = new Optional<void*>(idp);
				}
				else
				{
					rowIdp = new Optional<void*>();
				}

				//TODO : Create pointers, Optional, etc.
				_nativeRangeBound = new fs::RangeBound(valuep, *rowIdp, inclusive);
			}

			ManagedRangeBound()
			{
				_nativeRangeBound = new fs::RangeBound();
			}

			~ManagedRangeBound()
			{
				if (idp != NULL)
					delete idp;

				if (valuep != NULL)
					delete valuep;

				if (rowIdp != NULL)
					delete rowIdp;
			}
	};

	public ref class ManagedRange
	{
		const static int MaxLimit = 500;

		private:
			fs::Range* _nativeRange;
			Optional<fs::RangeBound>*  startOpt;
			Optional<fs::RangeBound>*  endOpt;

		public:
			fs::Range* GetNativePointer();

			ManagedRange(fs::Range* nativeRange) : _nativeRange(nativeRange) {};
			ManagedRange(System::Int32 limit,  System::Boolean ascending, ManagedRangeBound^ start, ManagedRangeBound^ end)
			{		
				if (start != nullptr)
				{
					startOpt = new Optional<fs::RangeBound>(*(start->GetNativePointer()));
				}
				else
				{
					startOpt = new Optional<fs::RangeBound>();
				}

				if (end != nullptr)
				{
					endOpt = new Optional<fs::RangeBound>(*(end->GetNativePointer()));
				}
				else
				{
					endOpt = new Optional<fs::RangeBound>();
				}

				_nativeRange = new fs::Range(limit, *startOpt, *endOpt, ascending);
			}

			~ManagedRange()
			{
				if (startOpt != NULL)
					delete startOpt;

				if (endOpt != NULL)
					delete endOpt;
			}
	};
}