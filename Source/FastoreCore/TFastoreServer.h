#ifndef _THRIFT_SERVER_TFASTORESERVER_H_
#define _THRIFT_SERVER_TFASTORESERVER_H_ 1

#include <thrift/Thrift.h>
#include <thrift/server/TServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>
#include <climits>
#include <stack>
#include <vector>
#include <string>
#include <errno.h>
#include <cstdlib>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <poll.h>

namespace apache { namespace thrift { namespace server {

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::transport::TSocket;
using apache::thrift::protocol::TProtocol;

#ifndef SOCKOPT_CAST_T
#   ifndef _WIN32
#       define SOCKOPT_CAST_T void
#   else
#       define SOCKOPT_CAST_T char
#   endif // _WIN32
#endif

template<class T>
inline const SOCKOPT_CAST_T* const_cast_sockopt(const T* v)
{
	return reinterpret_cast<const SOCKOPT_CAST_T*>(v);
}

template<class T>
inline SOCKOPT_CAST_T* cast_sockopt(T* v)
{
	return reinterpret_cast<SOCKOPT_CAST_T*>(v);
}

/**
* This is a non-blocking server (unsing non-blocking sockets) in C++, with only one thread. It assumes that
* all incoming requests are framed with a 4 byte length indicator and
* writes out responses using the same framing.
*
* It does not use the TServerTransport framework, but rather has socket
* operations hardcoded for use with select.
*
*/


/// Overload condition actions.
enum TOverloadAction
{
	T_OVERLOAD_NO_ACTION,        ///< Don't handle overload */
	T_OVERLOAD_CLOSE_ON_ACCEPT  ///< Drop new connections immediately */
};

/// Three states for sockets: recv frame size, recv data, and send mode
enum TSocketState
{
	SOCKET_RECV_FRAMING,
	SOCKET_RECV,
	SOCKET_SEND
};

/**
* Five states for the nonblocking server:
*  1) initialize
*  2) read 4 byte frame size
*  3) read frame of data
*  4) send back data (if any)
*  5) force immediate connection close
*/
enum TAppState
{
	APP_INIT,
	APP_READ_FRAME_SIZE,
	APP_READ_REQUEST,
	APP_PARKED,
	APP_SEND_RESULT,
	APP_CLOSE_CONNECTION
};

enum TServerState
{
	SERVER_STARTUP,
	SERVER_SHUTDOWN,
	SERVER_OVERLOADED,
	SERVER_RUN,
	SERVER_STOP,
	SERVER_TRANSITION
};


class TFastoreServer : public TServer
{
public:
	class TConnection;

private:

	/// Listen backlog
	static const int LISTEN_BACKLOG = 1024;

	/// Default limit on size of idle connection pool
	static const size_t CONNECTION_POOL_LIMIT = 1024;

	/// Default limit on frame size
	static const int MAX_FRAME_SIZE = 256 * 1024 * 1024;

	/// Default limit on total number of connected sockets
	static const int MAX_CONNECTIONS = 1024;

	/// Default size of write buffer
	static const int WRITE_BUFFER_DEFAULT_SIZE = 1024;

	/// Maximum size of read buffer allocated to idle connection (0 = unlimited)
	static const int IDLE_READ_BUFFER_LIMIT = 1024;

	/// Maximum size of write buffer allocated to idle connection (0 = unlimited)
	static const int IDLE_WRITE_BUFFER_LIMIT = 1024;

	/// # of calls before resizing oversized buffers (0 = check only on close)
	static const int RESIZE_BUFFER_EVERY_N = 512;

	/// File descriptor of an invalid socket
	static const int INVALID_SOCKET_VALUE = -1;

	/// Default poll timeout
	static const int POLL_TIMEOUT = 1000;

	/// Default number of milliseconds a connections is idle before it timesout
	static const int CONNECTION_EXPIRE_TIMEOUT = 10000;

	/// Default number of milliseconds to wait for connections to finish before force-closing them
	static const int SHUTDOWN_TIMEOUT = 5000;

	/// Server socket file descriptor
	int serverSocket_;

	/// Port server runs on
	int port_;

	/// Limit for how many TConnection objects to cache
	size_t connectionPoolLimit_;

	/// Limit for number of open connections
	size_t maxConnections_;

	/// Limit for frame size
	size_t maxFrameSize_;

	/// Time in milliseconds before a connection expires (0 == infinite).
	int64_t connectionExpireTime_;

	/// number of milliseconds to poll for events
	int pollTimeout_;

	/**
	* Hysteresis for overload state.  This is the fraction of the overload
	* value that needs to be reached before the overload state is cleared;
	* must be <= 1.0.
	*/
	double overloadHysteresis_;

	/// Action to take when we're overloaded.
	TOverloadAction overloadAction_;

	/**
	* The write buffer is initialized (and when idleWriteBufferLimit_ is checked
	* and found to be exceeded, reinitialized) to this size.
	*/
	size_t writeBufferDefaultSize_;

	/**
	* Max read buffer size for an idle TConnection.  When we place an idle
	* TConnection into connectionStack_ or on every resizeBufferEveryN_ calls,
	* we will free the buffer (such that it will be reinitialized by the next
	* received frame) if it has exceeded this limit.  0 disables this check.
	*/
	size_t idleReadBufferLimit_;

	/**
	* Max write buffer size for an idle connection.  When we place an idle
	* TConnection into connectionStack_ or on every resizeBufferEveryN_ calls,
	* we insure that its write buffer is <= to this size; otherwise we
	* replace it with a new one of writeBufferDefaultSize_ bytes to insure that
	* idle connections don't hog memory. 0 disables this check.
	*/
	size_t idleWriteBufferLimit_;

	/**
	* Every N calls we check the buffer size limits on a connected TConnection.
	* 0 disables (i.e. the checks are only done when a connection closes).
	*/
	int32_t resizeBufferEveryN_;

	/// Set if we are currently in an overloaded state.
	bool overloaded_;

	/// Count of connections dropped since overload started
	uint32_t nConnectionsDropped_;

	/// Count of connections dropped on overload since server started
	uint64_t nTotalConnectionsDropped_;

	//Pool of UNUSED connections
	std::stack<TConnection*> connectionPool_;

	//List of Active connections (by socket fd id).
	std::map<int, TConnection*> activeConnections_;

	//List of connections to close when no longer in use
	std::vector<int> closePool_;

	// Signal to stop (immediate close)
	bool _stop;

	// Signal to shutdown (don't accept new connections, try to finish current connections, and then stop)
	bool _shutdown;

	// Time is milliseconds a server will continue to run to finish serving connections before it forcibly terminates.
	int64_t _shutdownTimeout;

	// Marks the time the shutdown started. We may want to rethink this so that shutdown is not based on a timeout, but rather some other criteria.
	clock_t _shutdownStart;

	// Descriptors for sockets to poll. Size must be set upon a transition, or startup.
	pollfd* _fds;

	void init(int port)
	{
		serverSocket_ = -1;
		port_ = port;
		connectionPoolLimit_ = CONNECTION_POOL_LIMIT;  
		maxConnections_ = MAX_CONNECTIONS;
		maxFrameSize_ = MAX_FRAME_SIZE;
		connectionExpireTime_ = CONNECTION_EXPIRE_TIMEOUT;
		overloadHysteresis_ = 0.8;
		overloadAction_ = T_OVERLOAD_NO_ACTION;
		writeBufferDefaultSize_ = WRITE_BUFFER_DEFAULT_SIZE;
		idleReadBufferLimit_ = IDLE_READ_BUFFER_LIMIT;
		idleWriteBufferLimit_ = IDLE_WRITE_BUFFER_LIMIT;
		resizeBufferEveryN_ = RESIZE_BUFFER_EVERY_N;
		pollTimeout_ = POLL_TIMEOUT;
		_shutdownTimeout = SHUTDOWN_TIMEOUT;
		overloaded_ = false;
		_stop = false;
		_shutdown = false;
		nConnectionsDropped_ = 0;
		nTotalConnectionsDropped_ = 0;
		_fds = NULL;
	}

public:
	template<typename ProcessorFactory>
	TFastoreServer
		(
			const boost::shared_ptr<ProcessorFactory>& processorFactory,
			int port,
			THRIFT_OVERLOAD_IF(ProcessorFactory, TProcessorFactory)
		) :
		TServer(processorFactory)
		{
			init(port);
		}

	template<typename Processor>
	TFastoreServer
		(const boost::shared_ptr<Processor>& processor,
			int port,
			THRIFT_OVERLOAD_IF(Processor, TProcessor)
		) :
		TServer(processor)
		{
			init(port);
		}

	template<typename ProcessorFactory>
	TFastoreServer
		(
			const boost::shared_ptr<ProcessorFactory>& processorFactory,
			const boost::shared_ptr<TProtocolFactory>& protocolFactory,
			int port,
			THRIFT_OVERLOAD_IF(ProcessorFactory, TProcessorFactory)
		) : 
		TServer(processorFactory)
		{

			init(port);

			setInputProtocolFactory(protocolFactory);
			setOutputProtocolFactory(protocolFactory);
		}

	template<typename Processor>
	TFastoreServer
		(
			const boost::shared_ptr<Processor>& processor,
			const boost::shared_ptr<TProtocolFactory>& protocolFactory,
			int port,
			THRIFT_OVERLOAD_IF(Processor, TProcessor)
		) :
		TServer(processor)
		{
			init(port);

			setInputProtocolFactory(protocolFactory);
			setOutputProtocolFactory(protocolFactory);
		}

	template<typename ProcessorFactory>
	TFastoreServer
		(
			const boost::shared_ptr<ProcessorFactory>& processorFactory,
			const boost::shared_ptr<TTransportFactory>& inputTransportFactory,
			const boost::shared_ptr<TTransportFactory>& outputTransportFactory,
			const boost::shared_ptr<TProtocolFactory>& inputProtocolFactory,
			const boost::shared_ptr<TProtocolFactory>& outputProtocolFactory,
			int port,
			THRIFT_OVERLOAD_IF(ProcessorFactory, TProcessorFactory)
		) :
		TServer(processorFactory)
		{
			init(port);

			setInputTransportFactory(inputTransportFactory);
			setOutputTransportFactory(outputTransportFactory);
			setInputProtocolFactory(inputProtocolFactory);
			setOutputProtocolFactory(outputProtocolFactory);
		}

	template<typename Processor>
	TFastoreServer
		(
			const boost::shared_ptr<Processor>& processor,
			const boost::shared_ptr<TTransportFactory>& inputTransportFactory,
			const boost::shared_ptr<TTransportFactory>& outputTransportFactory,
			const boost::shared_ptr<TProtocolFactory>& inputProtocolFactory,
			const boost::shared_ptr<TProtocolFactory>& outputProtocolFactory,
			int port,
			THRIFT_OVERLOAD_IF(Processor, TProcessor)
		) :
		TServer(processor)
		{
			init(port);

			setInputTransportFactory(inputTransportFactory);
			setOutputTransportFactory(outputTransportFactory);
			setInputProtocolFactory(inputProtocolFactory);
			setOutputProtocolFactory(outputProtocolFactory);
		}

	~TFastoreServer();

	// Get the current number of milliseconds for polling.
	int getPollTimeout() const
	{
		return pollTimeout_;
	}

	// Set the number of milliseconds to poll for.
	void setPollTimeout(int timeout)
	{
		pollTimeout_ = timeout;
	}

	// Get the maximum number of unused TConnection we will hold in reserve.
	size_t getConnectionPoolLimit() const
	{
		return connectionPoolLimit_;
	}

	/**
	* Set the maximum number of unused TConnection we will hold in reserve.
	*
	* @param sz the new limit for TConnection pool size.
	*/
	void setConnectionPoolLimit(size_t sz)
	{
		connectionPoolLimit_ = sz;
	}

	/**
	* Return the count of sockets currently connected to.
	*
	* @return count of connected sockets.
	*/
	size_t getNumConnections() const
	{
		return activeConnections_.size();
	}

	/**
	* Return the count of sockets currently connected to.
	*
	* @return count of connected sockets.
	*/
	size_t getNumActiveConnections() const
	{
		return getNumConnections() - getNumIdleConnections();
	}

	/**
	* Return the count of connection objects allocated but not in use.
	*
	* @return count of idle connection objects.
	*/
	size_t getNumIdleConnections() const
	{
		return connectionPool_.size();
	}

	/**
	* Get the maximum # of connections allowed before overload.
	*
	* @return current setting.
	*/
	size_t getMaxConnections() const
	{
		return maxConnections_;
	}

	/**
	* Set the maximum # of connections allowed before overload.
	*
	* @param maxConnections new setting for maximum # of connections.
	*/
	void setMaxConnections(size_t maxConnections)
	{
		maxConnections_ = maxConnections;
	}

	/**
	* Get the maximum allowed frame size.
	*
	* If a client tries to send a message larger than this limit,
	* its connection will be closed.
	*
	* @return Maxium frame size, in bytes.
	*/
	size_t getMaxFrameSize() const
	{
		return maxFrameSize_;
	}

	/**
	* Set the maximum allowed frame size.
	*
	* @param maxFrameSize The new maximum frame size.
	*/
	void setMaxFrameSize(size_t maxFrameSize)
	{
		maxFrameSize_ = maxFrameSize;
	}

	/**
	* Get fraction of maximum limits before an overload condition is cleared.
	*
	* @return hysteresis fraction
	*/
	double getOverloadHysteresis() const
	{
		return overloadHysteresis_;
	}

	/**
	* Set fraction of maximum limits before an overload condition is cleared.
	* A good value would probably be between 0.5 and 0.9.
	*
	* @param hysteresisFraction fraction <= 1.0.
	*/
	void setOverloadHysteresis(double hysteresisFraction) 
	{
		if (hysteresisFraction <= 1.0 && hysteresisFraction > 0.0)
		{
			overloadHysteresis_ = hysteresisFraction;
		}
	}

	/**
	* Get the action the server will take on overload.
	*
	* @return a TOverloadAction enum value for the currently set action.
	*/
	TOverloadAction getOverloadAction() const
	{
		return overloadAction_;
	}

	/**
	* Set the action the server is to take on overload.
	*
	* @param overloadAction a TOverloadAction enum value for the action.
	*/
	void setOverloadAction(TOverloadAction overloadAction)
	{
		overloadAction_ = overloadAction;
	}

	/**
	* Get the time in milliseconds after which a connection expires (0 == infinite).
	*
	* @return a 64-bit time in milliseconds.
	*/
	int64_t getconnectionExpireTime() const
	{
		return connectionExpireTime_;
	}

	/**
	* Set the time in milliseconds after which a connection expires (0 == infinite).
	*
	* @param connectionExpireTime a 64-bit time in milliseconds.
	*/
	void setconnectionExpireTime(int64_t connectionExpireTime)
	{
		connectionExpireTime_ = connectionExpireTime;
	}

	/**
	* Get the time in milliseconds after which a shutdown starts the server will forcibly terminate
	*
	* @return a 64-bit time in milliseconds.
	*/
	int64_t getShutdownTimeout() const
	{
		return _shutdownTimeout;
	}

	/**
	* Set the time in milliseconds after which a connection expires (0 == infinite).
	*
	* @param shutdownTimeout a 64-bit time in milliseconds.
	*/
	void setShutdownTimeout(int64_t shutdownTimeout)
	{
		_shutdownTimeout = shutdownTimeout;
	}
	
	/**
	* Determine if the server is currently overloaded.
	* This function checks the maximums for open connections and connections
	* currently in processing, and sets an overload condition if they are
	* exceeded.  The overload will persist until both values are below the
	* current hysteresis fraction of their maximums.
	*
	* @return true if an overload condition exists, false if not.
	*/
	bool serverOverloaded();

	/**
	* Get the starting size of a TConnection object's write buffer.
	*
	* @return # bytes we initialize a TConnection object's write buffer to.
	*/
	size_t getWriteBufferDefaultSize() const
	{
		return writeBufferDefaultSize_;
	}

	/**
	* Set the starting size of a TConnection object's write buffer.
	*
	* @param size # bytes we initialize a TConnection object's write buffer to.
	*/
	void setWriteBufferDefaultSize(size_t size)
	{
		writeBufferDefaultSize_ = size;
	}

	/**
	* Get the maximum size of read buffer allocated to idle TConnection objects.
	*
	* @return # bytes beyond which we will dealloc idle buffer.
	*/
	size_t getIdleReadBufferLimit() const 
	{
		return idleReadBufferLimit_;
	}

	/**
	* Set the maximum size read buffer allocated to idle TConnection objects.
	* If a TConnection object is found (either on connection close or between
	* calls when resizeBufferEveryN_ is set) with more than this much memory
	* allocated to its read buffer, we free it and allow it to be reinitialized
	* on the next received frame.
	*
	* @param limit of bytes beyond which we will shrink buffers when checked.
	*/
	void setIdleReadBufferLimit(size_t limit)
	{
		idleReadBufferLimit_ = limit;
	}

	/**
	* Get the maximum size of write buffer allocated to idle TConnection objects.
	*
	* @return # bytes beyond which we will reallocate buffers when checked.
	*/
	size_t getIdleWriteBufferLimit() const
	{
		return idleWriteBufferLimit_;
	}

	/**
	* Set the maximum size write buffer allocated to idle TConnection objects.
	* If a TConnection object is found (either on connection close or between
	* calls when resizeBufferEveryN_ is set) with more than this much memory
	* allocated to its write buffer, we destroy and construct that buffer with
	* writeBufferDefaultSize_ bytes.
	*
	* @param limit of bytes beyond which we will shrink buffers when idle.
	*/
	void setIdleWriteBufferLimit(size_t limit)
	{
		idleWriteBufferLimit_ = limit;
	}

	/**
	* Get # of calls made between buffer size checks.  0 means disabled.
	*
	* @return # of calls between buffer size checks.
	*/
	int32_t getResizeBufferEveryN() const
	{
		return resizeBufferEveryN_;
	}

	/**
	* Check buffer sizes every "count" calls.  This allows buffer limits
	* to be enforced for persistant connections with a controllable degree
	* of overhead. 0 disables checks except at connection close.
	*
	* @param count the number of calls between checks, or 0 to disable
	*/
	void setResizeBufferEveryN(int32_t count)
	{
		resizeBufferEveryN_ = count;
	}

	/**
	* Main workhorse function, starts up the server listening on a port and
	* loops over the libevent handler.
	*/
	void serve();

	/**
	* Causes the server to terminate immediately (can be called from any thread).
	*/
	void stop();

	/**
	* Causes the server to terminate gracefully (can be called from any thread).
	*/
	void shutdown();	

	/**
	* Return whether or not the server is in a shutdown state
	*/
	bool isShuttingDown();
	

private:

	/// Creates a socket to listen on and binds it to the local port.
	void createAndListenOnSocket();

	/**
	*Causes the server to terminate all connections that haven't had activity. Timeout
	*set by connectionTimeout.
	*/
	void expireConnections();

	/**
	* Takes a socket created by createAndListenOnSocket() and sets various
	* options on it to prepare for use in the server.
	*
	* @param fd descriptor of socket to be initialized/
	*/
	void listenSocket(int fd);
	/**
	* Return an initialized connection object.  Creates or recovers from
	* pool a TConnection and initializes it with the provided socket FD
	* and flags.
	*
	* @param socket FD of socket associated with this connection.
	* @param addr the sockaddr of the client
	* @param addrLen the length of addr
	* @return pointer to initialized TConnection object.
	*/
	TConnection* createConnection(int socket, const sockaddr* addr,
		socklen_t addrLen);

	/**
	* Returns a connection to pool (or deletes it if pool is full).
	*/
	void returnConnection(int socket);

	void markForClose(int socket);

	void closeMarkedConnections();

	//Run connection loop
	void run();

	//accept new connections
	void acceptConnections();

};

/**
* TConnection
*/

/**
* Represents a connection that is handled via libevent. This connection
* essentially encapsulates a socket that has some associated libevent state.
*/
class TFastoreServer::TConnection
{
private:

	/// Server handle
	TFastoreServer* server_;

	/// TProcessor
	boost::shared_ptr<TProcessor> processor_;

	/// Object wrapping network socket
	boost::shared_ptr<TSocket> tSocket_;

	/// Socket mode
	TSocketState socketState_;

	/// Application state
	TAppState appState_;

	/// How much data needed to read
	uint32_t readWant_;

	/// Where in the read buffer are we
	uint32_t readBufferPos_;

	/// Read buffer
	uint8_t* readBuffer_;

	/// Read buffer size
	uint32_t readBufferSize_;

	/// Write buffer
	uint8_t* writeBuffer_;

	/// Write buffer size
	uint32_t writeBufferSize_;

	/// How far through writing are we?
	uint32_t writeBufferPos_;

	/// Largest size of write buffer seen since buffer was constructed
	size_t largestWriteBufferSize_;

	/// Count of the number of calls for use with getResizeBufferEveryN().
	int32_t callsForResize_;

	// Time last action occured on the connection
	clock_t lastAction_;

	/// Transport to read from
	boost::shared_ptr<TMemoryBuffer> inputTransport_;

	/// Transport that processor writes to
	boost::shared_ptr<TMemoryBuffer> outputTransport_;

	/// extra transport generated by transport factory (e.g. BufferedRouterTransport)
	boost::shared_ptr<TTransport> factoryInputTransport_;
	boost::shared_ptr<TTransport> factoryOutputTransport_;

	/// Protocol decoder
	boost::shared_ptr<TProtocol> inputProtocol_;

	/// Protocol encoder
	boost::shared_ptr<TProtocol> outputProtocol_;

	/// Server event handler, if any
	boost::shared_ptr<TServerEventHandler> serverEventHandler_;

	/// Thrift call context, if any
	void *connectionContext_;

	/// Close this connection and free or reset its resources.
	void close();

public:

	/// Constructor
	TConnection
		(
			int socket, TFastoreServer* server,
			const sockaddr* addr, socklen_t addrLen
		)
		{
			readBuffer_ = NULL;
			readBufferSize_ = 0;

			server_ = server;

			// Allocate input and output transports these only need to be allocated
			// once per TConnection (they don't need to be reallocated on init() call)
			inputTransport_.reset(new TMemoryBuffer(readBuffer_, readBufferSize_));
			outputTransport_.reset(new TMemoryBuffer(server_->getWriteBufferDefaultSize()));
			tSocket_.reset(new TSocket());
			init(socket, server, addr, addrLen);
		}

	~TConnection()
	{
		std::free(readBuffer_);
	}

	/**
	* Check buffers against any size limits and shrink it if exceeded.
	*
	* @param readLimit we reduce read buffer size to this (if nonzero).
	* @param writeLimit if nonzero and write buffer is larger, replace it.
	*/
	void checkIdleBufferMemLimit(size_t readLimit, size_t writeLimit);

	/// Initialize
	void init(int socket, TFastoreServer* server, const sockaddr* addr, socklen_t addrLen);

	/**
	* This is called when the application transitions from one state into
	* another. This means that it has finished writing the data that it needed
	* to, or finished receiving the data that it needed to.
	*/
	void transition();

	/**
	* server calls this when socket is ready
	*/
	void workSocket();

	// When transitioning if the connection flagged to be parked
	// The transition will go to park rather than return after an operation.
	void park()
	{
		appState_ = APP_PARKED;
	}

	//Attempt to expire the connection. If the last action occured
	// more than timeout milliseconds ago, close the connection.
	void expire(long timeout)
	{
		if ((clock() - lastAction_) > timeout)
			appState_ = APP_CLOSE_CONNECTION;
	}

	/// Force connection shutdown for this connection.
	void forceClose()
	{
		appState_ = APP_CLOSE_CONNECTION;
		close();
	}

	/// return the server this connection was initialized for.
	TFastoreServer* getServer() const
	{
		return server_;
	}

	/// get state of connection.
	TAppState getState() const
	{
		return appState_;
	}

	/// get state of socket.
	TSocketState getSocketState() const
	{
		return socketState_;
	}

	/// return the TSocket transport wrapping this network connection
	boost::shared_ptr<TSocket> getTSocket() const
	{
		return tSocket_;
	}

	/// return the server event handler if any
	boost::shared_ptr<TServerEventHandler> getServerEventHandler()
	{
		return serverEventHandler_;
	}

	/// return the Thrift connection context if any
	void* getConnectionContext()
	{
		return connectionContext_;
	}

	TFastoreServer* getServer()
	{
		return server_;
	}

};

}}} // apache::thrift::server

#endif // #ifndef _THRIFT_SERVER_TFASTORESERVER_H_
