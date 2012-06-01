#pragma once

#include <string>
#include <fstream>
#include <memory>

using namespace std;

class Log
{
private:
	string _fileName;
	auto_ptr<fstream> _file;
	long _latestRevision;
public:
	Log(string fileName);

	long getLatestRevision() { return _latestRevision; }
};
