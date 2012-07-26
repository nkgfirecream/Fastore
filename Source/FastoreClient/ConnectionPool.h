#pragma once

#include "ClientException.h"
#include <map>
#include <vector>
#include <queue>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Collections::Generic;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Linq;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Text;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Threading;
using namespace Thrift::Protocol;

namespace Alphora
{
	namespace Fastore
	{
		namespace Client
		{
			template<typename TKey, typename TClient>
			class ConnectionPool
			{
			public:
				static const int MaxConnectionRetries = 3;
				static const int DefaultMaxPooledPerKey = 3;

			private:
				boost::shared_ptr<object> _lock;
				std::map<TKey, std::queue<TClient>*> _entries;
				Func<TProtocol*, TClient> _createConnection;
				Func<TKey, NetworkAddress*> _determineAddress;
				Action<TClient> _destroyConnection;
				Func<TClient, bool> _isValid;

			public:
				ConnectionPool(Func<TProtocol*, TClient> createConnection, Func<TKey, NetworkAddress*> determineAddress, Action<TClient> destroyConnection, Func<TClient, bool> isValid)
				{
					InitializeInstanceFields();
					_createConnection = createConnection;
					_determineAddress = determineAddress;
					_destroyConnection = destroyConnection;
					_isValid = isValid;
				}

				~ConnectionPool()
				{
					InitializeInstanceFields();
//C# TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
					lock (_lock)
					{
						if (_entries.size() > 0)
						{
							auto errors = std::vector<std::exception>();
							for (std::map<TKey, std::queue<TClient>*>::const_iterator entry = _entries.begin(); entry != _entries.end(); ++entry)
							{
								for (unknown::const_iterator connection = entry->Value.begin(); connection != entry->Value.end(); ++connection)
									try
									{
										Destroy(*connection);
									}
									catch (std::exception &e)
									{
										errors->Add(e);
									}
							}
							_entries.clear();
							ClientException::ThrowErrors(errors);
						}
					}
				}

			private:
				int _maxPooledPerKey;
			public:
				const int &getMaxPooledPerKey() const
				{
					return _maxPooledPerKey;
				}
				void setMaxPooledPerKey(const int &value)
				{
					_maxPooledPerKey = value;
				}

				TClient operator [](TKey key)
				{
					Monitor::Enter(_lock);
					bool taken = true;
					try
					{
						// Loop to throw away invalid connections
						while (true)
						{
							// Check for existing known worker
							std::queue<TClient> entry;
							if (!_entries.TryGetValue(key, entry))
							{
								// Release the lock during connection
								Monitor::Exit(_lock);
								taken = false;

								auto address = _determineAddress(key);

								return Connect(address);
							}
							else
							{
								auto result = entry.pop();

								// If last one out, remove the entry
								if (entry.size()() == 0)
									_entries.erase(key);

								if (_isValid(result))
									return result;
							}
						}
					}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
					finally
					{
						if (taken)
							Monitor::Exit(_lock);
					}
				}

				void Release(KeyValuePair<TKey, TClient> connection)
				{
//C# TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
					lock (_lock)
					{
						// Find or create the entry
						std::queue<TClient> entry;
						if (!_entries.TryGetValue(connection.Key, entry))
						{
							entry = std::queue<TClient>();
							_entries.insert(make_pair(connection.Key, entry));
						}

						entry.push(connection.Value);

						// If limit exceeded, throw away old connection(s) as needed
						while (entry.size()() > _maxPooledPerKey)
							Destroy(entry.pop());
					}
				}

				void Destroy(TClient connection)
				{
					_destroyConnection(connection);
				}

				/// <summary> Makes a connection to a service given address information. </summary>
				/// <remarks> The resulting connection will not yet be in the pool.  Use release to store the connection in the pool. </remarks>
				TClient Connect(const boost::shared_ptr<NetworkAddress> &address)
				{
					auto transport = boost::make_shared<Thrift::Transport::TSocket>(address->Name, address->Port);

					// Establish connection, retrying if necessary
					auto retries = MaxConnectionRetries;
					while (true)
						try
						{
							retries--;
							transport->Open();
							break;
						}
						catch (System::Net::Sockets::SocketException *e)
						{
							if (retries == 0)
								throw;
						}

					try
					{
						auto bufferedTransport = boost::make_shared<Thrift::Transport::TFramedTransport>(transport);
						auto protocol = boost::make_shared<Thrift::Protocol::TBinaryProtocol>(bufferedTransport);

						return _createConnection(protocol);
					}
					catch (...)
					{
						transport->Close();
						throw;
					}
				}

			private:
				void InitializeInstanceFields()
				{
					_lock = boost::make_shared<object>();
					_entries = std::map<TKey, std::queue<TClient>*>();
					_maxPooledPerKey = DefaultMaxPooledPerKey;
				}
			};
		}
	}
}
