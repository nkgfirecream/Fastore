#include "Module.h"
#include "CModule.h"
#include <cassert>

using namespace std;

static string errorMessage;

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

void intializeFastoreModule(sqlite3* db, int argc, void* argv)
{
	if( argc < 1 || argv == NULL ) {
		assert(argc > 0);
		errorMessage = "argc == 0";
		return;
	}
	try {
		//Convert from c to cpp...
		std::vector<module::Address> mas(1);
		module::Address& address( mas.front() );

		const char * input = reinterpret_cast<const char *>(argv);
		istringstream reader(input);
		std::getline(reader, address.Name, ';');
		address.Port = -1;
		reader >> address.Port;
		if( address.Port == -1 ) {
			ostringstream msg;
			msg << "could not parse port from: " << input;
			errorMessage = msg.str(); 
			return;
		}

		intializeFastoreModule(db, mas);
	}
	catch( const std::exception& oops ) {
		errorMessage = oops.what();
	}
	catch( ... ) {
		errorMessage = "exceptional exception";
	}
}
