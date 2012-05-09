#pragma once
#include "typedefs.h"

struct TransactionID
{
	private:
		long long _id;

	public:	
		long long GetRevision()
		{
			return (_id >> 16);
		}

		void SetRevision(long long revision)
		{
			_id = (revision << 16) | (_id & 0x000000000000FFFF);
		}

		short GetSequence()
		{
			return (short)_id;
		}

		void SetSequence(short transaction)
		{
			_id = transaction | (_id & 0xFFFFFFFFFFFF0000);
		}
};