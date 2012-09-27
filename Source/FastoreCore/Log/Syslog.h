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
	std::string errstr;
	std::ostringstream msg;
	int errnum, priority, env_priority;
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
		// capture errno if we're starting a new log message. 
		if( msg.tellp() == 0 && errstr.empty() && errno != 0 ) {
			errnum = errno;
			errstr = strerror(errno);
		} 
	
		msg << input;

		return *this;
	}

};

extern Syslog Log;	

} // end namespace 
