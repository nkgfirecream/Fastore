// $Id$
#include "Syslog.h"
#include <cstdlib>

using namespace std;

namespace fastore {

#if _WIN32
	static std::string ident;
	static 	int priority(-1);

	static void openlog(const char *ident, int option, int facility) 
	{
		if( ident )
			::fastore::ident = ident;

		const char *s = getenv("SYSLOG");
		if( s  ) {
			std::istringstream is( s );
			is >> priority;
		}
	}
	static void closelog(void) {}

	static void syslog(int priority, const char *, const char *msg)
	{
		if( priority & ::fastore::priority )
			if( ! ident.empty() ) {
				std::string tmp( ident ); 
				tmp += ": ";
				OutputDebugStringA( tmp.c_str() );
			}
			OutputDebugStringA( msg );
	}
#endif

	Syslog::Syslog( const char *ident, int option, int facility )
		: errnum(-1)
		, priority(facility)
	{
		openlog( ident, option, facility );
	}

	Syslog::~Syslog()
	{
		closelog();
	}
	
	Syslog&
	Syslog::endl( Syslog& )
	{
		std::string msg( this->msg.str() );
		size_t pos = msg.find("%m");

		// robustness would look for every %m, not just one
		if( pos != std::string::npos && pos > 0 && msg[pos-1] != '%' ) {
			std::ostringstream e;
			e << errnum;
			msg.replace( pos, 2, e.str() );
		}

		syslog( priority, "%s", msg.c_str() );
		this->msg.str( std::string() );

		return *this;
	}


} // end namespace 
