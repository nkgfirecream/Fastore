using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using Thrift.Protocol;

namespace Alphora.Fastore.Client
{
	public class ConnectionPool<TKey, TClient> : IDisposable
	{
		public const int MaxConnectionRetries = 3;
		public const int DefaultMaxPooledPerKey = 3;

		private Object _lock = new Object();
		private Dictionary<TKey, Queue<TClient>> _entries = new Dictionary<TKey, Queue<TClient>>();
		private Func<TProtocol, TClient> _createConnection;
		private Func<TKey, NetworkAddress> _determineAddress;
		private Action<TClient> _destroyConnection;
		private Func<TClient, bool> _isValid;

		public ConnectionPool
		(
			Func<TProtocol, TClient> createConnection, 
			Func<TKey, NetworkAddress> determineAddress, 
			Action<TClient> destroyConnection,
			Func<TClient, bool> isValid
		)
		{
			_createConnection = createConnection;
			_determineAddress = determineAddress;
			_destroyConnection = destroyConnection;
			_isValid = isValid;
		}

		public void Dispose()
		{
			lock (_lock)
			{
				if (_entries != null)
				{
					var errors = new List<Exception>();
					foreach (var entry in _entries)
					{
						foreach (var connection in entry.Value)
							try
							{
								Destroy(connection);
							}
							catch (Exception e)
							{
								errors.Add(e);
							}
					}
					_entries = null;
					ClientException.ThrowErrors(errors);
				}
			}
		}

		private int _maxPooledPerKey = DefaultMaxPooledPerKey;
		public int MaxPooledPerKey
		{
			get { return _maxPooledPerKey; }
			set { _maxPooledPerKey = value; }
		}

		public TClient this[TKey key]
		{
			get
			{
				Monitor.Enter(_lock);
				bool taken = true;
				try
				{
					// Loop to throw away invalid connections
					while (true)
					{
						// Check for existing known worker
						Queue<TClient> entry;
						if (!_entries.TryGetValue(key, out entry))
						{
							// Release the lock during connection
							Monitor.Exit(_lock);
							taken = false;

							var address = _determineAddress(key);

							return Connect(address);
						}
						else
						{
							var result = entry.Dequeue();

							// If last one out, remove the entry
							if (entry.Count() == 0)
								_entries.Remove(key);

							if (_isValid(result))
								return result;
						}
					}
				}
				finally
				{
					if (taken)
						Monitor.Exit(_lock);
				}
			}
		}

		public void Release(KeyValuePair<TKey, TClient> connection)
		{
            lock (_lock)
            {
				// Find or create the entry
				Queue<TClient> entry;
                if (!_entries.TryGetValue(connection.Key, out entry))
				{	
					entry = new Queue<TClient>();
                    _entries.Add(connection.Key, entry);
				}

				entry.Enqueue(connection.Value);

				// If limit exceeded, throw away old connection(s) as needed
				while (entry.Count() > _maxPooledPerKey)
					Destroy(entry.Dequeue());
            }
		}

		public void Destroy(TClient connection)
		{
			_destroyConnection(connection);
		}

		/// <summary> Makes a connection to a service given address information. </summary>
		/// <remarks> The resulting connection will not yet be in the pool.  Use release to store the connection in the pool. </remarks>
		public TClient Connect(NetworkAddress address)
		{
			var transport = new Thrift.Transport.TSocket(address.Name, address.Port);

			// Establish connection, retrying if necessary
			var retries = MaxConnectionRetries;
			while (true)
				try
				{
					retries--;
					transport.Open();
					break;
				}
				catch (System.Net.Sockets.SocketException e)
				{
					if (retries == 0)
						throw;
				}

			try
			{
				var bufferedTransport = new Thrift.Transport.TFramedTransport(transport);
				var protocol = new Thrift.Protocol.TBinaryProtocol(bufferedTransport);

				return _createConnection(protocol);
			}
			catch
			{
				transport.Close();
				throw;
			}
		}
	}
}
