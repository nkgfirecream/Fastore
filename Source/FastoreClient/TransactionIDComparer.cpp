#include "TransactionIDComparer.h"

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

const boost::shared_ptr<TransactionIDComparer> TransactionIDComparer::Default = boost::make_shared<TransactionIDComparer>();

			int TransactionIDComparer::Compare(const boost::shared_ptr<TransactionID> &x, const boost::shared_ptr<TransactionID> &y)
			{
				return x->Revision.compare(y->Revision);
			}
		}
	}
}
