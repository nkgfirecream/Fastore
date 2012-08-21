#pragma once

namespace fastore
{
	namespace provider
	{
		enum ArgumentType
		{
			FASTORE_ARGUMENT_NULL,
			FASTORE_ARGUMENT_DOUBLE,
			FASTORE_ARGUMENT_INT32,
			FASTORE_ARGUMENT_INT64,
			FASTORE_ARGUMENT_STRING8,
			FASTORE_ARGUMENT_STRING16,
			FASTORE_ARGUMENT_BOOL
		};
	}
}