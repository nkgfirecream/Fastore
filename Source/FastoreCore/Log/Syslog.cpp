// $Id$
#define SYSLOG_NAMES
#include "Syslog.h"
#include <cstdlib>
#include <algorithm>

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

	static void syslog(int priority, const char *, const char *msg)
	{
		const CODE& code = *std::find_if(prioritynames, 
										 prioritynames + sizeof(prioritynames)/sizeof(*prioritynames), 
										 [&] ( const CODE& c) {
											return c.c_val == priority || c.c_val == -1;
										 } ); 

		ostringstream os;
		os << "[" <<  (code.c_name? code.c_name : "") << "] " << ident << ": " << msg;

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
	}

	Syslog::~Syslog()
	{
		closelog();
	}
	
	Syslog&
	Syslog::endl( Syslog& )
	{
		if( (env_priority & priority) == 0 ) 
			return *this;

		std::string msg( this->msg.str() );
		size_t pos = msg.find("%m");

		// robustness would look for every %m, not just one
		if( pos != std::string::npos && pos > 0 && msg[pos-1] != '%' ) {
			msg.replace( pos, 2, errstr );
		}

		msg += "\n";
		
		syslog( priority, "%s", msg.c_str() );
		
		this->msg.str( std::string() );
		errstr.clear();

		return *this;
	}


	Syslog Log;
} // end namespace 
