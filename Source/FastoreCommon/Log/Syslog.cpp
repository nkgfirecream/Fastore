// $Id: Syslog.cpp 679 2012-10-26 01:03:38Z jpercival $
#if _WIN32
# define SYSLOG_NAMES
#endif
#include <cstdlib>
#include <algorithm>

#include "Syslog.h"

using namespace std;

namespace fastore {

#if _WIN32
	static std::string ident;
	static 	int priority(-1);

	static void openlog(const char *ident, int option, int facility) 
	{
		if( ident )
			::fastore::ident = ident;
	}

	static void closelog(void) {}

	void syslog(int priority, const char *, const char *msg)
	{
		const CODE& code = *std::find_if(prioritynames, 
										 prioritynames + sizeof(prioritynames)/sizeof(*prioritynames), 
										 [&] ( const CODE& c) {
											return c.c_val == priority || c.c_val == -1;
										 } ); 

		ostringstream os;
		os << ident << ": " <<  (code.c_name? code.c_name : "") << ": " << msg;

		OutputDebugStringA( os.str().c_str() );
	}
#endif

	Syslog::Syslog( const char *ident, int option, int facility )
		: priority(facility)
		, env_priority(0)
		, errnum(0)
	{
		openlog( ident, option, facility );

		const char *s = getenv("FASTORE_SYSLOG");
		if( s  ) {
			std::istringstream is( s );
			is >> env_priority;
		}

		msg << boolalpha;
	}

	Syslog::~Syslog()
	{
		closelog();
	}
	
	Syslog&
	log_endl( Syslog& log)
	{
		if( (log.env_priority & log.priority) == 0 ) 
			return log;

		std::string msg( log.msg.str() );
		size_t pos = msg.find("%m");

		// robustness would look for every %m, not just one
		if( pos != std::string::npos && pos > 0 && msg[pos-1] != '%' ) {
			msg.replace( pos, 2, log.errstr );
		}

		msg += "\n";
		
		syslog( log.priority, "%s", msg.c_str() );
		
		log.msg.str( std::string() );
		log.errstr.clear();

		return log;
	}

	Syslog& log_emerg( Syslog& log )   
	{ 
		log.priority = LOG_EMERG;
		return log; 
	}
	Syslog& log_alert( Syslog& log )   
	{ 
		log.priority = LOG_ALERT;
		return log; 
	}
	Syslog& log_crit ( Syslog& log )   
	{ 
		log.priority = LOG_CRIT;
		return log; 
	}
	Syslog& log_err( Syslog& log )     
	{ 
		log.priority = LOG_ERR;
		return log; 
	}
	Syslog& log_warning( Syslog& log ) 
	{ 
		log.priority = LOG_WARNING; 
		return log; 
	}
	Syslog& log_notice( Syslog& log )  
	{ 
		log.priority = LOG_NOTICE;  
		return log; 
	}
	Syslog& log_info ( Syslog& log )   
	{ 
		log.priority = LOG_INFO;    
		return log; 
	}
	Syslog& log_debug( Syslog& log )   
	{ 
		log.priority = LOG_DEBUG;   
		return log; 
	}

	Syslog Log;

void
write_log( const char *message ) 
{
	Log << message << log_endl;
}

} // end namespace 

fastore::Syslog& 
operator<<( fastore::Syslog& log, 
			fastore::Syslog& (*func)(fastore::Syslog& ) )
{
	return func(log);
}

fastore::Syslog& 
operator<<( fastore::Syslog& log, 
			const std::exception& err )
{
	return log << fastore::log_err << ": " << err.what();
}

// This needs a convenient place.  
// It could go in Communications/operators.cpp, but there's
// nowhere to declare it. 

//ostream& operator<<( ostream& os, const fastore::communication::NetworkAddress& addr ) 
//{
//	return os << "{" << addr.name << ":" << addr.port << "}";
//}

