#include "IDGenerator.h"
#include "../FastoreCore/Log/Syslog.h"

using namespace fastore::client;
using namespace std;
using fastore::Log;

IDGenerator::IDGenerator(std::function<int64_t(int)> _generateCallback, int blockSize, int allocationThreshold)
	: _generateCallback(_generateCallback), _blockSize(blockSize), _allocationThreshold(allocationThreshold), _loadingBlock(false), _endOfRange(0), _nextId(0)
{
	if (blockSize < 1)
		throw std::runtime_error("Block size must be at least 1.");
	if (allocationThreshold < 0)
		throw std::runtime_error("Allocation threshold must be 0 or more.");
	if (allocationThreshold > blockSize)
		throw std::runtime_error("Allocation threshold must be no more than the block size.");
}

void IDGenerator::AsyncGenerateBlock()
{
	try
	{
		// Fetch the next ID block
		int64_t blockValue = _generateCallback(_blockSize);
		_nextId = blockValue;
		_endOfRange = blockValue + _blockSize;
	}
	catch (std::exception &e)
	{
		Log << __func__ << e << log_endl;
		// If an error happens here, any waiting requesters will block
		throw e;
	}
}

int64_t IDGenerator::Generate()
{
	boost::lock_guard<boost::mutex> lock(_m);
	while(true)
	{
		// Deal with low number of IDs remaining
		auto remaining = _endOfRange - _nextId;
		if (remaining <= 0)
		{
			try
			{
				AsyncGenerateBlock();
			}
			catch(...)
			{
				continue;
			}

			return _nextId;
		}
		else
			return 	++_nextId;
	}	
}
