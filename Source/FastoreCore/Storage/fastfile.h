#pragma once

#include <Windows.h>
#include <EASTL\string.h>

using namespace std;

class FastFile
{
public:
	static DWORD _pageSize;

	// Static constructor
    static class _init
    {
        _init() 
		{ 
			// Determine the system page size
			SYSTEM_INFO info;
			GetSystemInfo(&info);
			_pageSize = info.dwPageSize;
		}
    } _initializer;

	FastFile(string fileName, bool isReadOnly);
	~FastFile();

	string getFileName();
	bool getIsReadOnly();
	bool isOpen();


private:
	string _fileName;
	bool _isReadOnly;

};
