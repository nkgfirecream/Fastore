#include "ConnectionPool.h"

using namespace fastore::client;

template<typename TKey, typename TClient>
ConnectionPool<TKey, TClient>::ConnectionPool(std::function<TClient(boost::shared_ptr<TProtocol>)> createConnection, std::function<NetworkAddress(TKey)> determineAddress, std::function<void(TClient&)> destroyConnection, std::function<bool(TClient&)> isValid)
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
ConnectionPool<TKey, TClient>::~ConnectionPool()
{
	_lock.lock();
	{
		if (_entries.size() > 0)
		{
			auto errors = std::vector<std::exception>();
			for (auto entry = _entries.begin(); entry != _entries.end(); ++entry)
			{
				for (auto connection = entry->second.begin(); connection != entry->second.end(); ++connection)
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
	_lock.unlock();
}

template<typename TKey, typename TClient>
const int ConnectionPool<TKey, TClient>::getMaxPooledPerKey()
{
	return _maxPooledPerKey;
}

template<typename TKey, typename TClient>
void ConnectionPool<TKey, TClient>::setMaxPooledPerKey(const int &value)
{
	_maxPooledPerKey = value;
}

template<typename TKey, typename TClient>
TClient& ConnectionPool<TKey, TClient>::operator [](TKey key)
{
	_lock.lock();
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
				_lock.unlock();
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
	catch(std::exception& e)
	{
		if (taken)
			_lock.unlock();
	}
}

template<typename TKey, typename TClient>
void ConnectionPool<TKey, TClient>::Release(std::pair<TKey, TClient> connection)
{
	_lock.lock();
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
	_lock.unlock();
}

template<typename TKey, typename TClient>
void ConnectionPool<TKey, TClient>::Destroy(TClient connection)
{
	_destroyConnection(connection);
}

template<typename TKey, typename TClient>
TClient ConnectionPool<TKey, TClient>::Connect(const NetworkAddress &address)
{
	auto transport = TSocket(address->Name, address->Port);

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