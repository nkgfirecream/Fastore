#include "IDGenerator.h"


using namespace fastore::client;

IDGenerator::IDGenerator(std::function<long long(long long)> _generateCallback, long long blockSize, int allocationThreshold)
	: _generateCallback(_generateCallback), _blockSize(blockSize), _allocationThreshold(allocationThreshold)
{
	if (blockSize < 1)
		throw std::exception("Block size must be at least 1.");
	if (allocationThreshold < 0)
		throw std::exception("Allocation threshold must be 0 or more.");
	if (allocationThreshold > blockSize)
		throw std::exception("Allocation threshold must be no more than the block size.");

	//Signals the io service to not shut down, since we will be continually
	//posting work.
	boost::asio::io_service::work work(_io_service);

	for (int i = 0; i < ThreadPoolSize; ++i)
	{
		//Set up a pool of threads so that the io service can process multiple "works"
		_threads.create_thread(boost::bind(&boost::asio::io_service::run, &_io_service));
	}
}

void IDGenerator::AsyncGenerateBlock()
{
	try
	{
		long long newBlock;
		try
		{
			// Fetch the next ID block
			newBlock = _generateCallback(_blockSize);
		}
		catch (std::exception &e)
		{
			// If an error happens here, any waiting requesters will block
			ResetLoading(boost::optional<long long>(), e);
			throw;
		}

		ResetLoading(newBlock, boost::optional<std::exception>());
	}
	catch (...)
	{
		// Don't ever let an exception leave a thread (kills process)
	}
}

void IDGenerator::ResetLoading(boost::optional<long long> newBlock, boost::optional<std::exception> e)
{
	// Take the latch
	
	while (!_spinlock.try_lock());
	bool taken = true;
	try
	{

		// Update the generation block data
		if (newBlock)
		{
			auto blockValue = *newBlock;
			_nextId = blockValue;
			_endOfRange = blockValue + _blockSize;
		}

		// Release the loading status
		_lastError = e;
		_loadingBlock = false;
		_loadEvent.set();
		_spinlock.unlock();
		taken = false;
	}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
	catch(std::exception& e)
	{
		if (taken)
			_spinlock.unlock();
	}
}

long long IDGenerator::Generate()
{
	while (true)
	{
		// Take ID generation latch
		while (!_spinlock.try_lock());
		bool lockTaken = true;
		try
		{
			// Generate
			long long nextId = _nextId;
			++_nextId;

			// Deal with low number of IDs remaining
			auto remaining = _endOfRange - _nextId;
			if (remaining < _allocationThreshold)
			{
				// If haven't begun loading the next block, commence
				if (!_loadingBlock)
				{
					_loadEvent.unset();
					_loadingBlock = true;
					_lastError = boost::optional<std::exception>();
					_io_service.post(boost::bind(&IDGenerator::AsyncGenerateBlock, this));
				}

				// Out of IDs?
				if (remaining < 0)
				{
					// Release latch
					if (lockTaken)
					{
						_spinlock.unlock();
						lockTaken = false;
					}

					// Wait for load to complete
					_loadEvent.wait();

					// Take the lock back
					while (!_spinlock.try_lock());
					lockTaken = true;

					// Throw if there was an error attempting to load a new block
					if (_lastError)
						throw std::exception(_lastError->what()); // Don't rethrow the exception instance, this would mutate exception state such as the stack information and this exception is shared across threads

					// Retry with new block
					continue;
				}
			}

			if(lockTaken)
				_spinlock.unlock();

			return nextId;
		}
		catch(std::exception& e)
		{
			// Release latch
			if (lockTaken)
				_spinlock.unlock();
		}
	}
}

void IDGenerator::ResetCache()
{
	while (true)
	{
		// Wait for any pending load event to complete
		_loadEvent.wait();

		// Take latch
		while (!_spinlock.try_lock());
		bool taken = true;
		try
		{
			// Ensure that loading didn't start before we took the latch

			if (!_loadEvent.wait_for(0))
			{
				continue;
			}

			// Reset ID generator
			_nextId = 0;
			_endOfRange = 0;

			_spinlock.unlock();
			taken = false;

			break;
		}
		catch(std::exception& e)
		{
			if(taken)
				_spinlock.unlock();
		}
	}
}

IDGenerator::~IDGenerator()
{
	//Reset work as well?
	_io_service.stop();
	_threads.join_all();
}