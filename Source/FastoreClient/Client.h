#pragma once

#include "Database.h"
#include "ServiceAddress.h"
#include <boost/shared_ptr.hpp>
#include "..\FastoreCommunication\Comm_types.h"

using namespace fastore::communication;

namespace fastore
{
	class Client
	{
	public:
		static boost::shared_ptr<Database> Connect(ServiceAddress addresses[]);
	};
}
