#pragma once
#include<sqlite3.h>

#ifdef __cplusplus 
extern "C" { 
#endif 

const char * getprogname();

const char * fastore_vfs_message(void);
bool initializeFastoreModule(sqlite3* db, int argc, void* argv);

#ifdef __cplusplus 
}
#endif 
