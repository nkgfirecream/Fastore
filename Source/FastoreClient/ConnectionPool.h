#pragma once

#include "ClientException.h"
#include <map>
#include <vector>
#include <queue>
#include <stdexcept>
#include <functional>
#include <boost/shared_ptr.hpp>
#include "../FastoreCommunication/Comm_types.h"
#include "../FastoreCore/safe_cast.h"
#include <thrift/protocol/TProtocol.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransport.h>
#include <thrift/transport/TBufferTransports.h>
#include <boost/thread/mutex.hpp>

using namespace fastore::communication;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

namespace fastore { namespace client
{
	template<typename TKey, typename TClient>
	class ConnectionPool
	{
	public:
		static const int MaxConnectionRetries = 3;
		static const int DefaultMaxPooledPerKey = 1;

	private:
		boost::shared_ptr<boost::mutex> _lock;
		std::map<TKey, std::queue<TClient>> _entries;

		std::function<TClient(boost::shared_ptr<TProtocol>)> _createConnection;
		std::function<NetworkAddress(TKey)> _determineAddress;
		std::function<void(TClient&)> _destroyConnection;
		std::function<bool(TClient&)> _isValid;


		size_t _maxPooledPerKey;
		void InitializeInstanceFields();

	public:
		ConnectionPool(std::function<TClient(boost::shared_ptr<TProtocol>)> createConnection, std::function<NetworkAddress(TKey)> determineAddress, std::function<void(TClient&)> destroyConnection, std::function<bool(TClient&)> isValid);
		~ConnectionPool();

		const size_t getMaxPooledPerKey();
		void setMaxPooledPerKey(const int &value);

		TClient operator[] (TKey key);
		void Release(std::pair<TKey, TClient> connection);
		void Destroy(TClient connection);

		/// <summary> Makes a connection to a service given address information. </summary>
		/// <remarks> The resulting connection will not yet be in the pool.  Use release to store the connection in the pool. </remarks>
		TClient Connect(const NetworkAddress &address);
	};



	template<typename TKey, typename TClient>
	ConnectionPool<TKey, TClient>::
	    ConnectionPool(std::function<TClient(boost::shared_ptr<TProtocol>)> createConnection, 
			   std::function<NetworkAddress(TKey)> determineAddress, 
			   std::function<void(TClient&)> destroyConnection, 
			   std::function<bool(TClient&)> isValid)
		: 
		_maxPooledPerKey(DefaultMaxPooledPerKey),
		_lock(boost::shared_ptr<boost::mutex>(new boost::mutex()))
	{
		_createConnection = createConnection;
		_determineAddress = determineAddress;
		_destroyConnection = destroyConnection;
		_isValid = isValid;
	}


	template<typename TKey, typename TClient>
	const size_t ConnectionPool<TKey, TClient>::getMaxPooledPerKey()
	{
		return _maxPooledPerKey;
	}

	template<typename TKey, typename TClient>
	void ConnectionPool<TKey, TClient>::setMaxPooledPerKey(const int &value)
	{
		_maxPooledPerKey = value;
	}

	template<typename TKey, typename TClient>
	TClient ConnectionPool<TKey, TClient>::operator[](TKey key)
	{
		_lock->lock();
		bool taken = true;
		try
		{
			// Loop to throw away invalid connections
			while (true)
			{
				// Check for existing known worker
				auto entry = _entries.find(key);
				if (entry == _entries.end())
				{
					// Release the lock during connection
					_lock->unlock();
					taken = false;

					auto address = _determineAddress(key);

					return Connect(address);
				}
				else
				{
					auto result = entry->second.front();
					entry->second.pop();

					// If last one out, remove the entry
					if (entry->second.size() == 0)
						_entries.erase(key);

					if (_isValid(result))
					{
						_lock->unlock();
						taken = false;
						return result;
					}
				}
			}
		}
		catch(std::exception& e)
		{
			if (taken)
				_lock->unlock();

			throw e;
		}
	}

	template<typename TKey, typename TClient>
	void ConnectionPool<TKey, TClient>::Release(std::pair<TKey, TClient> connection)
	{
		_lock->lock();
		{
			// Find or create the entry
			auto entry = _entries.find(connection.first);
			if (entry == _entries.end())
			{
				_entries.insert(std::pair<TKey, std::queue<TClient>>(connection.first, std::queue<TClient>()));
				entry = _entries.find(connection.first);
			}

			entry->second.push(connection.second);

			// If limit exceeded, throw away old connection(s) as needed
			while (entry->second.size() > _maxPooledPerKey)
			{
				Destroy(entry->second.front());
				entry->second.pop();
			}
		}
		_lock->unlock();
	}

	template<typename TKey, typename TClient>
	void ConnectionPool<TKey, TClient>::Destroy(TClient connection)
	{
		_destroyConnection(connection);
	}

	template<typename TKey, typename TClient>
	TClient ConnectionPool<TKey, TClient>::Connect(const NetworkAddress &address)
	{
		auto transport = boost::shared_ptr<TSocket>(
								new TSocket(address.name, 
											SAFE_CAST(int, address.port)));
		//TODO: Make this all configurable.
		transport->setConnTimeout(2000);
		transport->setRecvTimeout(2000);
		transport->setSendTimeout(2000);
		//transport->setMaxRecvRetries(MaxConnectionRetries);

		// Establish connection, retrying if necessary
		auto retries = MaxConnectionRetries;
		while (true)
			try
			{
				retries--;
				transport->open();
				break;
			}
			catch (const std::exception&)
			{
				if (retries == 0)
					throw;
			}

		try
		{
			auto bufferedTransport =  boost::shared_ptr<TTransport>(new TFramedTransport(transport));
			auto protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(bufferedTransport));

			return _createConnection(protocol);
		}
		catch (...)
		{
			transport->close();
			throw;
		}
	}

	template<typename TKey, typename TClient>
	ConnectionPool<TKey, TClient>::~ConnectionPool()
	{
		_lock->lock();
		{
			if (_entries.size() > 0)
			{
				std::vector<std::exception> errors;
				for (auto entry = _entries.begin(); entry != _entries.end(); ++entry)
				{
					while (entry->second.size() > 0)
					{
						auto connection = entry->second.front();

						try
						{
							Destroy(connection);
						}
						catch (std::exception &e)
						{
							errors.push_back(e);
						}

						entry->second.pop();
					}
				}
				_entries.clear();
				ClientException::ThrowErrors(errors);
			}
		}
		_lock->unlock();
	}
}}
