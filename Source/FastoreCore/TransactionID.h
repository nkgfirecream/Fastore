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
			//Clear upper 48 (keep lower 16)
			_id &= 0x000000000000FFFF;
			//Set upper 48
			_id |= (revision << 16);
		}

		short GetTransaction()
		{
			return (short)_id;
		}

		void SetTransaction(short transaction)
		{
			//Clear lower 16 bits (keep upper 48)
			_id &= ~0x000000000000FFFF;
			//Set lower 16 bits
			_id |= transaction;
		}
};