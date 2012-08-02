#include "IDGenerator.h"
#include <atomic>

using namespace fastore::client;

IDGenerator::IDGenerator(std::function<long long(long long)> _generateCallback, long long blockSize = DefaultBlockSize, int allocationThreshold = DefaultAllocationThreshold)
	: _generateCallback(_generateCallback), _blockSize(blockSize), _allocationThreshold(allocationThreshold)
{
	if (blockSize < 1)
		throw std::exception("Block size must be at least 1.");
	if (allocationThreshold < 0)
		throw std::exception("Allocation threshold must be 0 or more.");
	if (allocationThreshold > blockSize)
		throw std::exception("Allocation threshold must be no more than the block size.");

}

void IDGenerator::AsyncGenerateBlock(void*& state)
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
	while (Interlocked::CompareExchange(_generationLock, 1, 0) == 1);
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
		_loadingEvent->Set();
	}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
	finally
	{
		// Release latch
		Interlocked::Decrement(_generationLock);
	}
}

long long IDGenerator::Generate()
{
	while (true)
	{
		// Take ID generation latch
		while (Interlocked::CompareExchange(_generationLock, 1, 0) == 1);
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
					_loadingEvent->Reset();
					_loadingBlock = true;
					_lastError.smartpointerreset();
					ThreadPool::QueueUserWorkItem(boost::make_shared<WaitCallback>(AsyncGenerateBlock));
				}

				// Out of IDs?
				if (remaining < 0)
				{
					// Release latch
					if (lockTaken)
					{
						Interlocked::Decrement(_generationLock);
						lockTaken = false;
					}

					// Wait for load to complete
					_loadingEvent->WaitOne();

					// Take the lock back
					while (Interlocked::CompareExchange(_generationLock, 1, 0) == 1);
					lockTaken = true;

					// Throw if there was an error attempting to load a new block
					if (_lastError != nullptr)
						throw std::exception(_lastError.what()); // Don't rethrow the exception instance, this would mutate exception state such as the stack information and this exception is shared across threads

					// Retry with new block
					continue;
				}
			}

			return nextId;
		}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
		finally
		{
			// Release latch
			if (lockTaken)
				Interlocked::Decrement(_generationLock);
		}
	}
}

void IDGenerator::ResetCache()
{
	while (true)
	{
		// Wait for any pending load event to complete
		_loadingEvent->WaitOne();

		// Take latch
		while (Interlocked::CompareExchange(_generationLock, 1, 0) == 1);
		try
		{
			// Ensure that loading didn't start before we took the latch
			if (!_loadingEvent->WaitOne(0))
				continue;

			// Reset ID generator
			_nextId = 0;
			_endOfRange = 0;

			break;
		}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
		finally
		{
			Interlocked::Decrement(_generationLock);
		}
	}
}