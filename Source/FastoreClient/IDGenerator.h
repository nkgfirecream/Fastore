#pragma once

#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Threading;

namespace Alphora
{
	namespace Fastore
	{
		namespace Client
		{
			class IDGenerator
			{
				/// <summary> The default number of IDs to allocate with each batch. </summary>
			public:
				static const long long DefaultBlockSize = 100;

				/// <summary> The default low-water mark for starting allocation of next block. </summary>
				static const int DefaultAllocationThreshold = 5;

				/// <param name="generateCallback">Call-back method which is invoked when a new ID block is needed.  Should return the beginning of a block of the given size.  This call back will happen on a thread-pool thread. </param>
				/// <param name="blockSize">The number of IDs to allocate with each batch.  This is the most IDs that could potentially be "wasted" if not fully employed. </param>
				/// <param name="allocationThreshold">The low-water-mark for starting allocation of next block.</param>
				IDGenerator(GenerateIdCallback generateCallback, long long blockSize = DefaultBlockSize, int allocationThreshold = DefaultAllocationThreshold);

				// The size of block to allocate
			private:
				long long _blockSize;
				// The low-water-mark to start next block allocation
				int _allocationThreshold;
				// ID generation callback (when another block is needed)
				GenerateIdCallback _generateCallback;
				// Event which is unsignaled (blocking) while loading the next block
				boost::shared_ptr<ManualResetEvent> _loadingEvent;
				// True if we're in the process of loading another block of IDs					
				bool _loadingBlock;
				// Spin lock for in-memory protection ID allocation
				int _generationLock;
				// The next ID that should be allocated
				long long _nextId;
				// The end of the ID generation range (exclusive)			
				long long _endOfRange;
				// The last error that was thrown from allocation (to be relayed to requesters)					
				std::exception _lastError;


				/// <summary> Worker thread used to fetch the next block of IDs. </summary>
				/// <param name="state"> Unused (part of thread pool contract). </param>
				void AsyncGenerateBlock(const boost::shared_ptr<object> &state);

				/// <summary> Resets the loading state after attempting to load a new block.</summary>
				/// <param name="newBlock"> If the new block value is null, an error occurred. </param>
				void ResetLoading(Nullable<long long> newBlock, std::exception &e);

				/// <summary> Generates the next ID, either pulling from a preloaded block or by loading a new block.  </summary>
			public:
				long long Generate();

				/// <summary> Resets the ID generation cache so that the next request will hit the callback. </summary>
				void ResetCache();

				typedef long long (*GenerateIdCallback)(long long size);

			private:
				void InitializeInstanceFields();
			};
		}
	}
}
