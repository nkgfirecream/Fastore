#include "Module.h"
#include "CModule.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include "../FastoreCommon/Log/Syslog.h"

using namespace std;
using fastore::Log;
using fastore::log_endl;
using fastore::log_info;

static string errorMessage;


const char * getprogname() 
#if _WIN32
{ 
	static char name[MAX_PATH] = "GetModuleFileName";
	DWORD len =  GetModuleFileName( NULL, name, sizeof(name) );
	if( len ) {
		char *s = strrchr(name, '\\');
		if( s ) 
			return ++s;
	}
	return name;
}
#else
{ return program_invocation_short_name; }
#endif


const char * fastore_vfs_message(void)
{
	static char * msg(NULL);
	
	if( !errorMessage.empty() ) {
		delete[] msg;
		msg = new char[ 1 + errorMessage.size() ];
		strcpy( msg, errorMessage.c_str() );
		errorMessage.clear();
	}
	return msg;
}

void initializeFastoreModule(sqlite3* db, int argc, void* argv)
{
	if( argc < 1 || argv == NULL ) {
		assert(argc > 0);
		errorMessage = "argc == 0";
		return;
	}

	apache::thrift::GlobalOutput.setOutputFunction( fastore::write_log );

	Log << log_info << __func__ << " started" << log_endl;

	try {
		//Convert from c to cpp...
		std::vector<module::Address> mas(1);
		module::Address& address( mas.front() );

		const char * input = reinterpret_cast<const char *>(argv);
		istringstream reader(input);
		std::getline(reader, address.Name, ';');
		reader >> address.Port;
		if( reader.bad() || reader.fail() ) {
			ostringstream msg;
			msg << "could not parse port from: " << input;
			errorMessage = msg.str(); 
			return;
		}

		intializeFastoreModule(db, mas);
		//Log << log_info << __func__ << " Module initialized" << log_endl;
	}
	catch( const std::string& errorMessage ) {
		cerr << getprogname() << ": error:: " << errorMessage << endl;
		exit(EXIT_FAILURE);
	}
	catch( const std::exception& oops ) {
		errorMessage = oops.what();
		cerr << getprogname() << ": error: " << errorMessage << endl;
		exit(EXIT_FAILURE);
	}
	catch( ... ) {
		errorMessage = "exceptional exception";
		cerr << getprogname() << ": error: " << errorMessage << endl;
		exit(EXIT_FAILURE);
	}
}
