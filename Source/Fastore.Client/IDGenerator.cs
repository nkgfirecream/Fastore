using System;
using System.Threading;

namespace Alphora.Fastore.Client
{
	public class IDGenerator
	{
		/// <summary> The default number of IDs to allocate with each batch. </summary>
		public const long DefaultBlockSize = 100;

		/// <summary> The default low-water mark for starting allocation of next block. </summary>
		public const int DefaultAllocationThreshold = 5;	

		/// <param name="generateCallback">Call-back method which is invoked when a new ID block is needed.  Should return the beginning of a block of the given size.  This call back will happen on a thread-pool thread. </param>
		/// <param name="blockSize">The number of IDs to allocate with each batch.  This is the most IDs that could potentially be "wasted" if not fully employed. </param>
		/// <param name="allocationThreshold">The low-water-mark for starting allocation of next block.</param>
		public IDGenerator
		(
			GenerateIdCallback generateCallback,
			long blockSize = DefaultBlockSize,
			int allocationThreshold = DefaultAllocationThreshold
		)
		{
			if (blockSize < 1)
				throw new ArgumentOutOfRangeException("blockSize", "Block size must be at least 1.");
			if (allocationThreshold < 0)
				throw new ArgumentOutOfRangeException("allocationThreshold", "Allocation threshold must be 0 or more.");
			if (allocationThreshold > blockSize)
				throw new ArgumentOutOfRangeException("allocationThreshold", "Allocation threshold must be no more than the block size.");
			if (generateCallback == null)
				throw new ArgumentNullException("generateCallback");

			_generateCallback = generateCallback;
			_blockSize = blockSize;
			_allocationThreshold = allocationThreshold;
		}

		// The size of block to allocate
		private long _blockSize;
		// The low-water-mark to start next block allocation
		private int _allocationThreshold;
		// ID generation callback (when another block is needed)
		private GenerateIdCallback _generateCallback;
		// Event which is unsignaled (blocking) while loading the next block
		private ManualResetEvent _loadingEvent = new ManualResetEvent(true);
		// True if we're in the process of loading another block of IDs					
		private bool _loadingBlock;
		// Spin lock for in-memory protection ID allocation
		private int _generationLock;
		// The next ID that should be allocated
		private long _nextId;
		// The end of the ID generation range (exclusive)			
		private long _endOfRange;
		// The last error that was thrown from allocation (to be relayed to requesters)					
		private Exception _lastError;


		/// <summary> Worker thread used to fetch the next block of IDs. </summary>
		/// <param name="state"> Unused (part of thread pool contract). </param>
		private void AsyncGenerateBlock(object state)
		{
			try
			{
				long newBlock;
				try
				{
					// Fetch the next ID block
					newBlock = _generateCallback(_blockSize);
				}
				catch (Exception e)			
				{
					// If an error happens here, any waiting requesters will block
					ResetLoading(null, e);
					throw;
				}

				ResetLoading(newBlock, null);
			}
			catch
			{
				// Don't ever let an exception leave a thread (kills process)
			}
		}

		/// <summary> Resets the loading state after attempting to load a new block.</summary>
		/// <param name="newBlock"> If the new block value is null, an error occurred. </param>
		private void ResetLoading(long? newBlock, Exception e)
		{
			// Take the latch
			while (Interlocked.CompareExchange(ref _generationLock, 1, 0) == 1) ;
			try
			{
				// Update the generation block data
				if (newBlock != null)
				{
					var blockValue = newBlock.Value;
					_nextId = blockValue;
					_endOfRange = blockValue + _blockSize;
				}

				// Release the loading status
				_lastError = e;
				_loadingBlock = false;
				_loadingEvent.Set();
			}
			finally
			{
				// Release latch
				Interlocked.Decrement(ref _generationLock);
			}
		}

		/// <summary> Generates the next ID, either pulling from a preloaded block or by loading a new block.  </summary>
		public long Generate()
		{
			while (true)
			{
				// Take ID generation latch
				while (Interlocked.CompareExchange(ref _generationLock, 1, 0) == 1) ;
				bool lockTaken = true;
				try
				{
					// Generate
					long nextId = _nextId;
					++_nextId;

					// Deal with low number of IDs remaining
					var remaining = _endOfRange - _nextId;
					if (remaining < _allocationThreshold)
					{
						// If haven't begun loading the next block, commence
						if (!_loadingBlock)
						{
							_loadingEvent.Reset();
							_loadingBlock = true;
							_lastError = null;
							ThreadPool.QueueUserWorkItem(new WaitCallback(AsyncGenerateBlock));
						}

						// Out of IDs?
						if (remaining < 0)
						{
							// Release latch
							if (lockTaken)
							{
								Interlocked.Decrement(ref _generationLock);
								lockTaken = false;
							}

							// Wait for load to complete
							_loadingEvent.WaitOne();

							// Take the lock back
							while (Interlocked.CompareExchange(ref _generationLock, 1, 0) == 1) ;
							lockTaken = true;

							// Throw if there was an error attempting to load a new block
							if (_lastError != null)
								throw new Exception(_lastError.Message);	// Don't rethrow the exception instance, this would mutate exception state such as the stack information and this exception is shared across threads

							// Retry with new block
							continue;
						}
					}

					return nextId;
				}
				finally
				{
					// Release latch
					if (lockTaken)
						Interlocked.Decrement(ref _generationLock);
				}
			}
		}

		/// <summary> Resets the ID generation cache so that the next request will hit the callback. </summary>
		public void ResetCache()
		{
			while (true)
			{
				// Wait for any pending load event to complete
				_loadingEvent.WaitOne();
			
				// Take latch
				while (Interlocked.CompareExchange(ref _generationLock, 1, 0) == 1) ;
				try
				{
					// Ensure that loading didn't start before we took the latch
					if (!_loadingEvent.WaitOne(0))
						continue;

					// Reset ID generator
					_nextId = 0;
					_endOfRange = 0;

					break;
				}
				finally
				{
					Interlocked.Decrement(ref _generationLock);
				}
			}
		}

		public delegate long GenerateIdCallback(long size);
	}
}