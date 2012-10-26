#pragma once

#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/smart_ptr/detail/spinlock.hpp>
#include <boost/asio/io_service.hpp>

namespace fastore { namespace client
{
	class IDGenerator
	{

	public:
		/// <summary> The default number of IDs to allocate with each batch. </summary>
		static const int DefaultBlockSize = 5000; //Approx once per transaction...

		/// <summary> The default low-water mark for starting allocation of next block. </summary>
		static const int DefaultAllocationThreshold = 5;

		/// <param name="generateCallback">Call-back method which is invoked when a new ID block is needed.  Should return the beginning of a block of the given size.  This call back will happen on a thread-pool thread. </param>
		/// <param name="blockSize">The number of IDs to allocate with each batch.  This is the most IDs that could potentially be "wasted" if not fully employed. </param>
		/// <param name="allocationThreshold">The low-water-mark for starting allocation of next block.</param>
		IDGenerator(std::function<int64_t(int)> generateCallback, int blockSize = DefaultBlockSize, int allocationThreshold = DefaultAllocationThreshold);

		/// <summary> Generates the next ID, either pulling from a preloaded block or by loading a new block.  </summary>
		int64_t Generate();	

	private:
		// The size of block to allocate
		int _blockSize;
		// The low-water-mark to start next block allocation
		int _allocationThreshold;
		// ID generation callback (when another block is needed)		
		std::function<int64_t(int)> _generateCallback;

		// True if we're in the process of loading another block of IDs					
		bool _loadingBlock;
		// The next ID that should be allocated
		int64_t _nextId;
		// The end of the ID generation range (exclusive)			
		int64_t _endOfRange;
		// The last error that was thrown from allocation (to be relayed to requesters)					
		boost::optional<std::exception> _lastError;

		boost::mutex _m;

		//TODO: State was formerly and object.
		/// <summary> Worker thread used to fetch the next block of IDs. </summary>
		void AsyncGenerateBlock();
	};
}}
