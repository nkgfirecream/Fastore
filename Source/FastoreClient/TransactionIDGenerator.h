#pragma once

#include <stdint.h>
#include <boost/uuid/seed_rng.hpp>
#include <boost/random/mersenne_twister.hpp>

namespace fastore { namespace client
{

	class TransactionIDGenerator
	{
	private:
		boost::random::mt19937_64 _generator;
	public:
		TransactionIDGenerator(void)
		{
			boost::uuids::detail::seed(_generator);
		}

		int64_t generate()
		{
			return _generator.operator()();
		}

		~TransactionIDGenerator()
		{
		}
	};

}}