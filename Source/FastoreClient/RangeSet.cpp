#include "RangeSet.h"

//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Collections::Generic;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Linq;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Text;
namespace Alphora
{
	namespace Fastore
	{
		namespace Client
		{

			const bool &RangeSet::getEof() const
			{
				return privateEof;
			}

			void RangeSet::setEof(const bool &value)
			{
				privateEof = value;
			}

			const bool &RangeSet::getBof() const
			{
				return privateBof;
			}

			void RangeSet::setBof(const bool &value)
			{
				privateBof = value;
			}

			const bool &RangeSet::getLimited() const
			{
				return privateLimited;
			}

			void RangeSet::setLimited(const bool &value)
			{
				privateLimited = value;
			}

			const boost::shared_ptr<DataSet> &RangeSet::getData() const
			{
				return privateData;
			}

			void RangeSet::setData(const boost::shared_ptr<DataSet> &value)
			{
				privateData = value;
			}
		}
	}
}
