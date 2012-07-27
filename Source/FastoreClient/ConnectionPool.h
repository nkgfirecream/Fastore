#pragma once

#include "ClientException.h"
#include <map>
#include <vector>
#include <queue>
#include <stdexcept>
#include <functional>
#include <boost/shared_ptr.hpp>
#include "..\FastoreCommunication\Comm_types.h"
#include <thrift\protocol\TProtocol.h>


using namespace fastore::communication;
using namespace apache::thrift::protocol;

namespace fastore { namespace client
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

		std::function<TClient(TProtocol)> _createConnection;
		std::function<NetworkAddress(TKey)> _determineAddress;
		std::function<void(TClient)> _destroyConnection;
		std::function<bool(TClient)> _isValid;


		int _maxPooledPerKey;
		void InitializeInstanceFields();

	public:
		ConnectionPool(std::function<TClient(TProtocol)> createConnection, std::function<NetworkAddress(TKey)> determineAddress, std::function<void(TClient)> destroyConnection, std::function<bool(TClient)> isValid);
		~ConnectionPool();

		const int &getMaxPooledPerKey();
		void setMaxPooledPerKey(const int &value);

		TClient& operator[] (TKey key);
		void Release(std::pair<TKey, TClient> connection);
		void Destroy(TClient connection);

		/// <summary> Makes a connection to a service given address information. </summary>
		/// <remarks> The resulting connection will not yet be in the pool.  Use release to store the connection in the pool. </remarks>
		TClient Connect(const NetworkAddress &address);
	};
}}
