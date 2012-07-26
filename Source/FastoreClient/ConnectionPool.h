#pragma once

#include "ClientException.h"
#include <map>
#include <vector>
#include <queue>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include "..\FastoreCommunication\Comm_types.h"

using namespace fastore::communication;

namespace fastore
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
		int _maxPooledPerKey;
		void InitializeInstanceFields();

	public:
		ConnectionPool(Func<TProtocol*, TClient> createConnection, Func<TKey, NetworkAddress*> determineAddress, Action<TClient> destroyConnection, Func<TClient, bool> isValid);
		~ConnectionPool();

		const int &getMaxPooledPerKey();
		void setMaxPooledPerKey(const int &value);

		TClient operator [](TKey key);
		void Release(KeyValuePair<TKey, TClient> connection);
		void Destroy(TClient connection);

		/// <summary> Makes a connection to a service given address information. </summary>
		/// <remarks> The resulting connection will not yet be in the pool.  Use release to store the connection in the pool. </remarks>
		TClient Connect(const boost::shared_ptr<NetworkAddress> &address);
	};
}
