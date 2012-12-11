#pragma once
#include <map>
#include <vector>
#include <queue>
#include <stdexcept>
#include <functional>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include "LogStream.h"

class LogReaderPool
{
public:
	static const int DefaultMaxReadersPerKey = 3;

private:
	boost::shared_ptr<boost::mutex> _lock;

	std::map<int64_t, std::queue<std::shared_ptr<LogReader>>> _readers;

public:
	LogReaderPool(std::function<std::shared_ptr<LogReader>(int64_t)> create, std::function<bool(std::shared_ptr<LogReader>)> validate, std::function<void(std::shared_ptr<LogReader>)> destroy);
	~LogReaderPool();

	void Release(std::shared_ptr<LogReader> reader);
	void Destroy(std::shared_ptr<LogReader> reader);

	//Determine filename and validate.
	std::function<std::shared_ptr<LogReader>(int64_t)> _create;
	std::function<bool(std::shared_ptr<LogReader>&)> _validate;
	std::function<void(std::shared_ptr<LogReader>&)> _destroy;

	/// <summary> Makes a connection to a service given address information. </summary>
	/// <remarks> The resulting connection will not yet be in the pool.  Use release to store the connection in the pool. </remarks>
	std::shared_ptr<LogReader> LogReaderPool::operator[](int64_t lsn);

private:
	std::shared_ptr<LogReader> Open(int64_t lsn);
};