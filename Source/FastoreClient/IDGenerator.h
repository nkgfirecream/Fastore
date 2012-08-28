#pragma once

#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/smart_ptr/detail/spinlock.hpp>
#include <boost/asio/io_service.hpp>

namespace fastore { namespace client
{
	class IDGenerator
	{
	private:
		class manual_reset_event
		{
		public:
			manual_reset_event(bool signaled = true)
				: signaled_(signaled)
			{
			}

			void set()
			{
				{
					boost::unique_lock<boost::mutex> lock(m_);
					signaled_ = true;
				}

				// Notify all because until the event is manually
				// reset, all waiters should be able to see event signalling
				cv_.notify_all();
			}

			void unset()
			{
				boost::unique_lock<boost::mutex> lock(m_);
				signaled_ = false;
			}


			void wait()
			{
				boost::unique_lock<boost::mutex> lock(m_);
				while (!signaled_)
				{
					cv_.wait(lock);
				}
			}

			bool wait_for(long long milliseconds)
			{
				boost::unique_lock<boost::mutex> lock(m_);
				if (!signaled_)
				{					
					return cv_.timed_wait(lock, boost::posix_time::millisec(milliseconds));
				}
				else
					return true;
			}

		private:
			boost::mutex m_;
			boost::condition_variable cv_;
			bool signaled_;
		};


	public:
		/// <summary> The default number of IDs to allocate with each batch. </summary>
		static const int DefaultBlockSize = 100;


		/// <summary> The default low-water mark for starting allocation of next block. </summary>
		static const int DefaultAllocationThreshold = 5;

		/// <param name="generateCallback">Call-back method which is invoked when a new ID block is needed.  Should return the beginning of a block of the given size.  This call back will happen on a thread-pool thread. </param>
		/// <param name="blockSize">The number of IDs to allocate with each batch.  This is the most IDs that could potentially be "wasted" if not fully employed. </param>
		/// <param name="allocationThreshold">The low-water-mark for starting allocation of next block.</param>
		IDGenerator(std::function<int(int)> generateCallback, int blockSize = DefaultBlockSize, int allocationThreshold = DefaultAllocationThreshold);
		~IDGenerator();

		/// <summary> Generates the next ID, either pulling from a preloaded block or by loading a new block.  </summary>
		int Generate();

		/// <summary> Resets the ID generation cache so that the next request will hit the callback. </summary>
		void ResetCache();		

	private:
		//number of threads in the thread pool.
		static const int ThreadPoolSize = 4;

		// The size of block to allocate
		int _blockSize;
		// The low-water-mark to start next block allocation
		int _allocationThreshold;
		// ID generation callback (when another block is needed)		
		std::function<int(int)> _generateCallback;

		// True if we're in the process of loading another block of IDs					
		bool _loadingBlock;
		// The next ID that should be allocated
		int _nextId;
		// The end of the ID generation range (exclusive)			
		int _endOfRange;
		// The last error that was thrown from allocation (to be relayed to requesters)					
		boost::optional<std::exception> _lastError;


		//TODO: State was formerly and object.
		/// <summary> Worker thread used to fetch the next block of IDs. </summary>
		void AsyncGenerateBlock();

		//TODO: Nullable objects
		/// <summary> Resets the loading state after attempting to load a new block.</summary>
		/// <param name="newBlock"> If the new block value is null, an error occurred. </param>
		void ResetLoading(boost::optional<int> newBlock, boost::optional<std::exception> e);

		void InitializeInstanceFields();

		boost::asio::io_service _io_service;
		boost::thread_group _threads;
		manual_reset_event _loadEvent;
		boost::detail::spinlock _spinlock;

	};
}}
