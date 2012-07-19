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
inline const SOCKOPT_CAST_T* const_cast_sockopt(const T* v) {
  return reinterpret_cast<const SOCKOPT_CAST_T*>(v);
}

template<class T>
inline SOCKOPT_CAST_T* cast_sockopt(T* v) {
  return reinterpret_cast<SOCKOPT_CAST_T*>(v);
}

/**
 * This is a non-blocking server in C++ for high performance that
 * operates a set of IO threads (by default only one). It assumes that
 * all incoming requests are framed with a 4 byte length indicator and
 * writes out responses using the same framing.
 *
 * It does not use the TServerTransport framework, but rather has socket
 * operations hardcoded for use with select.
 *
 */


/// Overload condition actions.
enum TOverloadAction {
  T_OVERLOAD_NO_ACTION,        ///< Don't handle overload */
  T_OVERLOAD_CLOSE_ON_ACCEPT  ///< Drop new connections immediately */
};

class TFastoreServer : public TServer {
 private:
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

  
  // Signal to stop
  bool _break;

  void init(int port) {
    serverSocket_ = -1;
    port_ = port;
    connectionPoolLimit_ = CONNECTION_POOL_LIMIT;  
    maxConnections_ = MAX_CONNECTIONS;
    maxFrameSize_ = MAX_FRAME_SIZE;
    connectionExpireTime_ = 0;
    overloadHysteresis_ = 0.8;
    overloadAction_ = T_OVERLOAD_NO_ACTION;
    writeBufferDefaultSize_ = WRITE_BUFFER_DEFAULT_SIZE;
    idleReadBufferLimit_ = IDLE_READ_BUFFER_LIMIT;
    idleWriteBufferLimit_ = IDLE_WRITE_BUFFER_LIMIT;
    resizeBufferEveryN_ = RESIZE_BUFFER_EVERY_N;
	pollTimeout_ = POLL_TIMEOUT;
    overloaded_ = false;
    nConnectionsDropped_ = 0;
    nTotalConnectionsDropped_ = 0;
  }

 public:
  template<typename ProcessorFactory>
  TFastoreServer(
      const boost::shared_ptr<ProcessorFactory>& processorFactory,
      int port,
      THRIFT_OVERLOAD_IF(ProcessorFactory, TProcessorFactory)) :
    TServer(processorFactory) {
    init(port);
  }

  template<typename Processor>
  TFastoreServer(const boost::shared_ptr<Processor>& processor,
                     int port,
                     THRIFT_OVERLOAD_IF(Processor, TProcessor)) :
    TServer(processor) {
    init(port);
  }

  template<typename ProcessorFactory>
  TFastoreServer(
      const boost::shared_ptr<ProcessorFactory>& processorFactory,
      const boost::shared_ptr<TProtocolFactory>& protocolFactory,
      int port,
      THRIFT_OVERLOAD_IF(ProcessorFactory, TProcessorFactory)) :
    TServer(processorFactory) {

    init(port);

    setInputProtocolFactory(protocolFactory);
    setOutputProtocolFactory(protocolFactory);
  }

  template<typename Processor>
  TFastoreServer(
      const boost::shared_ptr<Processor>& processor,
      const boost::shared_ptr<TProtocolFactory>& protocolFactory,
      int port,
      THRIFT_OVERLOAD_IF(Processor, TProcessor)) :
    TServer(processor) {

    init(port);

    setInputProtocolFactory(protocolFactory);
    setOutputProtocolFactory(protocolFactory);
  }

  template<typename ProcessorFactory>
  TFastoreServer(
      const boost::shared_ptr<ProcessorFactory>& processorFactory,
      const boost::shared_ptr<TTransportFactory>& inputTransportFactory,
      const boost::shared_ptr<TTransportFactory>& outputTransportFactory,
      const boost::shared_ptr<TProtocolFactory>& inputProtocolFactory,
      const boost::shared_ptr<TProtocolFactory>& outputProtocolFactory,
      int port,
      THRIFT_OVERLOAD_IF(ProcessorFactory, TProcessorFactory)) :
    TServer(processorFactory) {

    init(port);

    setInputTransportFactory(inputTransportFactory);
    setOutputTransportFactory(outputTransportFactory);
    setInputProtocolFactory(inputProtocolFactory);
    setOutputProtocolFactory(outputProtocolFactory);
  }

  template<typename Processor>
  TFastoreServer(
      const boost::shared_ptr<Processor>& processor,
      const boost::shared_ptr<TTransportFactory>& inputTransportFactory,
      const boost::shared_ptr<TTransportFactory>& outputTransportFactory,
      const boost::shared_ptr<TProtocolFactory>& inputProtocolFactory,
      const boost::shared_ptr<TProtocolFactory>& outputProtocolFactory,
      int port,
      THRIFT_OVERLOAD_IF(Processor, TProcessor)) :
    TServer(processor) {

    init(port);

    setInputTransportFactory(inputTransportFactory);
    setOutputTransportFactory(outputTransportFactory);
    setInputProtocolFactory(inputProtocolFactory);
    setOutputProtocolFactory(outputProtocolFactory);
  }

  ~TFastoreServer();

    // Get the current number of milliseconds for polling.
  int getPollTimeout() const {
    return pollTimeout_;
  }

  // Set the number of milliseconds to poll for.
  void setPollTimeout(int timeout)
  {
    pollTimeout_ = timeout;
  }

  // Get the maximum number of unused TConnection we will hold in reserve.
  size_t getConnectionPoolLimit() const {
    return connectionPoolLimit_;
  }

  /**
   * Set the maximum number of unused TConnection we will hold in reserve.
   *
   * @param sz the new limit for TConnection pool size.
   */
  void setConnectionPoolLimit(size_t sz) {
    connectionPoolLimit_ = sz;
  }

  /**
   * Return the count of sockets currently connected to.
   *
   * @return count of connected sockets.
   */
  size_t getNumConnections() const {
    return activeConnections_.size();
  }

  /**
   * Return the count of sockets currently connected to.
   *
   * @return count of connected sockets.
   */
  size_t getNumActiveConnections() const {
    return getNumConnections() - getNumIdleConnections();
  }

  /**
   * Return the count of connection objects allocated but not in use.
   *
   * @return count of idle connection objects.
   */
  size_t getNumIdleConnections() const {
    return connectionPool_.size();
  }

  /**
   * Get the maximum # of connections allowed before overload.
   *
   * @return current setting.
   */
  size_t getMaxConnections() const {
    return maxConnections_;
  }

  /**
   * Set the maximum # of connections allowed before overload.
   *
   * @param maxConnections new setting for maximum # of connections.
   */
  void setMaxConnections(size_t maxConnections) {
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
  size_t getMaxFrameSize() const {
    return maxFrameSize_;
  }

  /**
   * Set the maximum allowed frame size.
   *
   * @param maxFrameSize The new maximum frame size.
   */
  void setMaxFrameSize(size_t maxFrameSize) {
    maxFrameSize_ = maxFrameSize;
  }

  /**
   * Get fraction of maximum limits before an overload condition is cleared.
   *
   * @return hysteresis fraction
   */
  double getOverloadHysteresis() const {
    return overloadHysteresis_;
  }

  /**
   * Set fraction of maximum limits before an overload condition is cleared.
   * A good value would probably be between 0.5 and 0.9.
   *
   * @param hysteresisFraction fraction <= 1.0.
   */
  void setOverloadHysteresis(double hysteresisFraction) {
    if (hysteresisFraction <= 1.0 && hysteresisFraction > 0.0) {
      overloadHysteresis_ = hysteresisFraction;
    }
  }

  /**
   * Get the action the server will take on overload.
   *
   * @return a TOverloadAction enum value for the currently set action.
   */
  TOverloadAction getOverloadAction() const {
    return overloadAction_;
  }

  /**
   * Set the action the server is to take on overload.
   *
   * @param overloadAction a TOverloadAction enum value for the action.
   */
  void setOverloadAction(TOverloadAction overloadAction) {
    overloadAction_ = overloadAction;
  }

  /**
   * Get the time in milliseconds after which a connection expires (0 == infinite).
   *
   * @return a 64-bit time in milliseconds.
   */
  int64_t getconnectionExpireTime() const {
    return connectionExpireTime_;
  }

  /**
   * Set the time in milliseconds after which a connection expires (0 == infinite).
   *
   * @param connectionExpireTime a 64-bit time in milliseconds.
   */
  void setconnectionExpireTime(int64_t connectionExpireTime) {
    connectionExpireTime_ = connectionExpireTime;
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
  size_t getWriteBufferDefaultSize() const {
    return writeBufferDefaultSize_;
  }

  /**
   * Set the starting size of a TConnection object's write buffer.
   *
   * @param size # bytes we initialize a TConnection object's write buffer to.
   */
  void setWriteBufferDefaultSize(size_t size) {
    writeBufferDefaultSize_ = size;
  }

  /**
   * Get the maximum size of read buffer allocated to idle TConnection objects.
   *
   * @return # bytes beyond which we will dealloc idle buffer.
   */
  size_t getIdleReadBufferLimit() const {
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
  void setIdleReadBufferLimit(size_t limit) {
    idleReadBufferLimit_ = limit;
  }

  /**
   * Get the maximum size of write buffer allocated to idle TConnection objects.
   *
   * @return # bytes beyond which we will reallocate buffers when checked.
   */
  size_t getIdleWriteBufferLimit() const {
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
  void setIdleWriteBufferLimit(size_t limit) {
    idleWriteBufferLimit_ = limit;
  }

  /**
   * Get # of calls made between buffer size checks.  0 means disabled.
   *
   * @return # of calls between buffer size checks.
   */
  int32_t getResizeBufferEveryN() const {
    return resizeBufferEveryN_;
  }

  /**
   * Check buffer sizes every "count" calls.  This allows buffer limits
   * to be enforced for persistant connections with a controllable degree
   * of overhead. 0 disables checks except at connection close.
   *
   * @param count the number of calls between checks, or 0 to disable
   */
  void setResizeBufferEveryN(int32_t count) {
    resizeBufferEveryN_ = count;
  }

  /**
   * Main workhorse function, starts up the server listening on a port and
   * loops over the libevent handler.
   */
  void serve();

  /**
   * Causes the server to terminate gracefully (can be called from any thread).
   */
  void stop();

  /**
   *Causes the server to terminate all connections that haven't had activity in the last
   *timeout milliseconds
   */
  void expire(long timeout);

 private:

  /// Creates a socket to listen on and binds it to the local port.
  void createAndListenOnSocket();

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
   * Returns a connection to pool or deletion.  If the connection pool
   * (a stack) isn't full, place the connection object on it, otherwise
   * just delete it.
   *
   * @param connection the TConection being returned.
   */
  void returnConnection(TConnection* connection);

  // Used by TConnection objects to indicate processing has finished.
  bool notify(TFastoreServer::TConnection* conn);

  //Run connection loop
  void run();

  //accept new connections
  void acceptConnections();

};

}}} // apache::thrift::server

#endif // #ifndef _THRIFT_SERVER_TFASTORESERVER_H_
