#pragma once
#include<sqlite3.h>

#ifdef __cplusplus 
extern "C" { 
#endif 

char * getprogname();

const char * fastore_vfs_message(void);
void intializeFastoreModule(sqlite3* db, int argc, void* argv);

#ifdef __cplusplus 
}
#endif 
