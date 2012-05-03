#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Range.h"
#pragma managed(pop)

#include "Utilities.h"

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
				valuep = Utilities::ConvertObjectToNative(value);

				Optional<void*>* rowIdp;

				if (rowId != nullptr)
				{
					idp = Utilities::ConvertObjectToNative(rowId);

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
			ManagedRange(System::Int32 column, System::Int32 limit, ManagedRangeBound^ start, ManagedRangeBound^ end)
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

				_nativeRange = new fs::Range(column, limit, *startOpt, *endOpt);
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