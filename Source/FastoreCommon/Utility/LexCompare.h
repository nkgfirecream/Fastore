#pragma once

#include <string>

#if defined(_WIN32)
# define strcasecmp(a,b) stricmp((a),(b))
# pragma warning( disable : 4996 )
#else
# include <strings.h>
#endif

using std::string;

struct LexCompare : public std::binary_function<string, string, bool> 
{
    bool operator()(const string &lhs, const string &rhs) const 
	{
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};
