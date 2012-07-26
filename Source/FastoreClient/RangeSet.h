#pragma once

#include "DataSet.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

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
			class RangeSet
			{
					private:
						bool privateEof;
					public:
						const bool &getEof() const;
						void setEof(const bool &value);
					private:
						bool privateBof;
					public:
						const bool &getBof() const;
						void setBof(const bool &value);
					private:
						bool privateLimited;
					public:
						const bool &getLimited() const;
						void setLimited(const bool &value);
					private:
						boost::shared_ptr<DataSet> privateData;
					public:
						const boost::shared_ptr<DataSet> &getData() const;
						void setData(const boost::shared_ptr<DataSet> &value);
			};
		}
	}
}
