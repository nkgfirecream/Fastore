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

		public:
			fs::RangeBound* GetNativePointer();

			ManagedRangeBound(System::Object^ value, System::Object^ rowId, System::Boolean inclusive)
			{
				//Convert object to void*
				//void* valuep = ConvertObjectToVoidPointer(value);

				Optional<void*>* rowIdp;

				if (rowId != nullptr)
				{
					//rowIdp = new Optional<void*>(ConvertObjectToVoidPointer(rowId));
				}
				else
				{
					rowIdp = new Optional<void*>();
				}

				//TODO : Create pointers, Optional, etc.
				//_nativeRangeBound = new fs::RangeBound(valuep, *rowIdp, inclusive);

				delete rowIdp;
			}

			ManagedRangeBound()
			{
				_nativeRangeBound = new fs::RangeBound();
			}
	};

	public ref class ManagedRange
	{
		const static int MaxLimit = 500;

		private:
			fs::Range* _nativeRange;

		public:
			fs::Range* GetNativePointer();

			ManagedRange(fs::Range* nativeRange) : _nativeRange(nativeRange) {};
			ManagedRange(System::Int32 limit, ManagedRangeBound^ start, ManagedRangeBound^ end, System::Boolean ascending)
			{
				Optional<fs::RangeBound>*  startOpt;
				Optional<fs::RangeBound>*  endOpt;

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

				delete startOpt;
				delete endOpt;
			}		
	};
}