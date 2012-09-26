// $Id$
#pragma once 
#if _WIN32
# include <windows.h>
# include "sys/syslog.h"
#else
# include <syslog.h>
#endif

#include <string>

#include <sstream>

namespace fastore {

class Syslog 
{
	int errnum;
	std::ostringstream msg;
	int priority;
public:
	Syslog( const char *ident = "Fastore", 
			int option = LOG_CONS|LOG_PID, 
			int facility = LOG_DAEMON );
	~Syslog();

	Syslog& endl( Syslog& );
	Syslog& operator<<( int input );

	template<typename T>
	Syslog& operator<<( const T& input ) 
	{
		msg << input;
		return *this;
	}

};

		

} // end namespace 
