#pragma once

#include <stdint.h>
#include <boost/uuid/seed_rng.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace fastore { namespace client
{

	class TransactionIDGenerator
	{
	private:
		boost::random::mt19937_64 _generator;
		boost::random::uniform_int_distribution<int64_t> _dist;
	public:
		TransactionIDGenerator(void)
		{
			boost::uuids::detail::seed(_generator);
		}

		int64_t generate()
		{
			return _dist(_generator);
		}

		~TransactionIDGenerator()
		{
		}
	};

}}