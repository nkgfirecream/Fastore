#pragma once

#include "DataSet.h"
#include <boost/shared_ptr.hpp>

namespace fastore { namespace client
{
	class RangeSet
	{
	private:
		bool _Eof;
		bool _Bof;
		bool _Limited;
		DataSet _Data;

	public:
		const bool &getEof() const;
		void setEof(const bool &value);
				
		const bool &getBof() const;
		void setBof(const bool &value);

		const bool &getLimited() const;
		void setLimited(const bool &value);

		DataSet& getData();
		void setData(DataSet &value);
	};
}}
