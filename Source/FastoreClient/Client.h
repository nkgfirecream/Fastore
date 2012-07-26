#pragma once

#include "Database.h"
#include "ServiceAddress.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace fastore
{
	class Client
	{
	public:
		static boost::shared_ptr<Database> Connect(ServiceAddress addresses[]);
	};
}
