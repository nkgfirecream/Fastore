#include "LogReaderPool.h"

LogReaderPool::LogReaderPool(std::function<std::shared_ptr<LogReader>(int64_t)> create, std::function<bool(std::shared_ptr<LogReader>)> validate, std::function<void(std::shared_ptr<LogReader>)> destroy)
	: 
	_lock(boost::shared_ptr<boost::mutex>(new boost::mutex())),
	_create(create),
	_validate(validate),
	_destroy(destroy)
{

}

std::shared_ptr<LogReader> LogReaderPool::operator[](int64_t lsn)
{
	_lock->lock();
	bool taken = true;
	try
	{
		// Loop to throw away invalid readers
		while (true)
		{
			// Check for existing known reader
			auto entry = _readers.find(lsn);
			if (entry == _readers.end())
			{
				// Release the lock during open
				_lock->unlock();
				taken = false;

				return Open(lsn);
			}
			else
			{
				auto result = entry->second.front();
				entry->second.pop();

				// If last one out, remove the entry
				if (entry->second.size() == 0)
					_readers.erase(lsn);

				//Checks to see if reader is up to date, updates if it can (size is out of date), returns false if it can't (log file has been destroyed)
				if (_validate(result))
				{
					_lock->unlock();
					taken = false;
					return result;
				}
				else
				{
					Destroy(result);
				}
			}
		}
	}
	catch(std::exception& e)
	{
		if (taken)
			_lock->unlock();

		throw e;
	}
}

void LogReaderPool::Release(std::shared_ptr<LogReader> reader)
{
	_lock->lock();
	{
		// Find or create the entry
		int64_t lsn = reader->lsn();

		auto entry = _readers.find(lsn);
		if (entry == _readers.end())
		{
			_readers.insert(std::pair<int64_t, std::queue<std::shared_ptr<LogReader>>>(lsn, std::queue<std::shared_ptr<LogReader>>()));
			entry = _readers.find(lsn);
		}

		entry->second.push(reader);

		// If limit exceeded, throw away old connection(s) as needed
		while (entry->second.size() > DefaultMaxReadersPerKey)
		{
			Destroy(entry->second.front());
			entry->second.pop();
		}
	}
	_lock->unlock();
}

void LogReaderPool::Destroy(std::shared_ptr<LogReader> reader)
{
	_destroy(reader);
}

std::shared_ptr<LogReader> LogReaderPool::Open(int64_t lsn)
{
	return _create(lsn);
}

LogReaderPool::~LogReaderPool()
{
	_lock->lock();
	{
		if (_readers.size() > 0)
		{
			std::vector<std::exception> errors;
			for (auto entry = _readers.begin(); entry != _readers.end(); ++entry)
			{
				while (entry->second.size() > 0)
				{
					auto reader = entry->second.front();

					try
					{
						Destroy(reader);
					}
					catch (std::exception &e)
					{
						errors.push_back(e);
					}

					entry->second.pop();
				}
			}
			_readers.clear();
		}
	}
	_lock->unlock();
}
