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

		private Object _lock = new Object();
		private Dictionary<TKey, TClient> _connections = new Dictionary<TKey, TClient>();
		private Func<TProtocol, TClient> _createConnection;
		private Func<TKey, NetworkAddress> _determineAddress;
		private Action<TClient> _destroyConnection;

		public ConnectionPool
		(
			Func<TProtocol, TClient> createConnection, 
			Func<TKey, NetworkAddress> determineAddress, 
			Action<TClient> destroyConnection
		)
		{
			_createConnection = createConnection;
			_determineAddress = determineAddress;
			_destroyConnection = destroyConnection;
		}

		public void Dispose()
		{
			lock (_lock)
			{
				if (_connections != null)
				{
					var errors = new List<Exception>();
					foreach (var connection in _connections.ToArray())
					{
						try
						{
							Destroy(connection.Value);
						}
						catch (Exception e)
						{
							errors.Add(e);
						}
						finally
						{
							_connections.Remove(connection.Key);
						}
					}
					ClientException.ThrowErrors(errors);
				}
				_connections = null;
			}
		}

		public TClient this[TKey key]
		{
			get
			{
				Monitor.Enter(_lock);
				bool taken = true;
				try
				{
					TClient result;
					// Check for existing known worker
					if (!_connections.TryGetValue(key, out result))
					{
						// Release the lock during connection
						Monitor.Exit(_lock);
						taken = false;

						var address = _determineAddress(key);

						result = Connect(address);
					}
					else
						_connections.Remove(key);

					return result;
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
			//Destroy(connection.Value);
            lock (_lock)
            {
                TClient oldClient;
                if (_connections.TryGetValue(connection.Key, out oldClient))
                {
                    Destroy(oldClient);
                    _connections[connection.Key] = connection.Value;
                }
                else
                    _connections.Add(connection.Key, connection.Value);
            }
		}

		public void Destroy(TClient connection)
		{
			_destroyConnection(connection);
		}

		/// <summary> Makes a connection to a service given address information. </summary>
		/// <remarks> The resulting connection will not be pooled.  Use release to store the connection in the pool. </remarks>
		public TClient Connect(NetworkAddress address)
		{
			var transport = new Thrift.Transport.TSocket(address.Name, address.Port);

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
