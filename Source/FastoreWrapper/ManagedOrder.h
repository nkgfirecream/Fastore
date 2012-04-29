#pragma managed(push, off)
#include "../FastoreCore/Order.h"
#pragma managed(pop)

#include "Utilities.h"

namespace Wrapper
{
	//TODO: These should probably be value types... 
	public ref class ManagedOrder
	{
		private:
			fs::Order* _nativeOrder;

		public:
			fs::Order* GetNativePointer();

			ManagedOrder()
			{
				_nativeOrder = new Order();
			}

			property System::Boolean Ascending
			{
				System::Boolean get()
				{
					return _nativeOrder->Ascending;
				}

				void set(System::Boolean value)
				{
					_nativeOrder->Ascending = value;
				}
			}

			property System::String^ Column
			{
				System::String^ get()
				{
					return Utilities::ConvertToManagedString(_nativeOrder->Column);
				}

				void set(System::String^ value)
				{
					_nativeOrder->Column = Utilities::ConvertToNativeWString(value);
				}
			}

	};
}