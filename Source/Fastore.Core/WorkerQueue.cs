using System;
using System.Collections.Generic;
using System.Threading;

namespace Fastore.Core
{
	/// <summary> Queues work onto a single worker thread. </summary>
	public class WorkerQueue
	{
		private Queue<System.Action> _asyncQueue = new Queue<System.Action>();
		private Thread _asyncThread;

		private ManualResetEventSlim _handle = new ManualResetEventSlim(true);
		public WaitHandle Handle { get { return _handle.WaitHandle; } }
		
		public WaitHandle Queue(System.Action action)
		{
			if (action != null)
			{
				var executeSync = false;

				lock (_asyncQueue)
				{
					_handle.Reset();
					if (_asyncThread == null)
					{
						_asyncThread = new Thread(new ThreadStart(AsyncQueueServiceThread));
						_asyncThread.Start();
					}
					else
						executeSync = (_asyncThread.ManagedThreadId == Thread.CurrentThread.ManagedThreadId);

					if (!executeSync)
						_asyncQueue.Enqueue(action);
				}

				if (executeSync)
				{
					action();
					_handle.Set();
				}
			}
			return _handle.WaitHandle;
		}

		private void AsyncQueueServiceThread()
		{
			while (true)
			{
				System.Action nextAction;
				lock (_asyncQueue)
				{
					if (_asyncQueue.Count > 0)
						nextAction = _asyncQueue.Dequeue();
					else
					{
						_asyncThread = null;
						_handle.Set();
						break;
					}
				}
				try
				{
					nextAction();
				}
				catch (Exception exception)
				{
					System.Diagnostics.Debug.WriteLine(exception.ToString());
					// Don't allow exceptions to leave this thread or the application will terminate
				}
			}
		}
	}
}
