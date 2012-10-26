// $Id$
#pragma once 
//Thrift defines it's own windows configuration (see next couple of lines)
//So on windows, it must be included first.
//#include "../../FastoreCommunication/Server_types.h"

#if _WIN32
# include <windows.h>
# include "sys/syslog.h"
#else
# include <syslog.h>
#endif

#include <cstring>
#include <string>
#include <sstream>

namespace fastore {

class Syslog 
{
	friend Syslog& log_endl( Syslog& );
	friend Syslog& log_emerg( Syslog& log );
	friend Syslog& log_alert( Syslog& log );
	friend Syslog& log_crit ( Syslog& log );
	friend Syslog& log_err( Syslog& log );
	friend Syslog& log_warning( Syslog& log );
	friend Syslog& log_notice( Syslog& log );
	friend Syslog& log_info ( Syslog& log );
	friend Syslog& log_debug( Syslog& log );

	std::string errstr;
	std::ostringstream msg;
	int errnum, priority, env_priority;
public:
	Syslog( const char *ident = "Fastore", 
			int option = LOG_CONS|LOG_PID, 
			int facility = LOG_DAEMON );
	~Syslog();

	template<typename T>
	Syslog& insert( const T& input ) 
	{
		// capture errno if we're starting a new log message. 
		if( msg.tellp() == std::streamoff(0) && errstr.empty() && errno != 0 ) {
			errnum = errno;
			errstr = strerror(errno);
		} 
	
		msg << input;

		return *this;
	}
};

Syslog& log_endl( Syslog& );
Syslog& log_emerg( Syslog& log );
Syslog& log_alert( Syslog& log );
Syslog& log_crit ( Syslog& log );
Syslog& log_err( Syslog& log );
Syslog& log_warning( Syslog& log );
Syslog& log_notice( Syslog& log );
Syslog& log_info ( Syslog& log );
Syslog& log_debug( Syslog& log );

extern Syslog Log;	

// created for thrift
void write_log( const char *message );

} // end namespace 

template <typename T>
fastore::Syslog& 
operator<<( fastore::Syslog& log, const T& input )
{
	return log.insert( input );
}

fastore::Syslog& 
operator<<( fastore::Syslog& log, 
			fastore::Syslog& (*func)(fastore::Syslog& ) );

fastore::Syslog& 
operator<<( fastore::Syslog& log, 
			const std::exception& err );

#if _WIN32
# define __func__ __FUNCTION__
#endif

